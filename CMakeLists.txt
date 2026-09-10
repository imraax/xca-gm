cmake_minimum_required(VERSION 3.13.0)

project(xca-gm
    DESCRIPTION "XCA-GM - X Certificate and Key management (GM/T)"
    HOMEPAGE_URL https://github.com/imraax/xca-gm
    LANGUAGES CXX
)

##### Product identity of XCA-GM (a fork of XCA by Christian Hohnstädt)
set(XCA_GM_NAME "XCA-GM")
set(XCA_GM_NAME_ZH "国密证书管理器")
set(XCA_GM_AUTHOR "RaaX")
set(XCA_GM_URL "https://github.com/imraax/xca-gm")
set(XCA_GM_APP_ID "io.github.imraax.xca-gm")
set(XCA_GM_COPYRIGHT "Copyright (C) 2026 RaaX. Based on XCA, Copyright (C) 2001 - 2024 Christian Hohnstädt")

file(READ VERSION.txt ver)
string(REGEX MATCH "([0-9\.]*)" _ ${ver})
set(PROJECT_VERSION ${CMAKE_MATCH_1})

cmake_host_system_information(RESULT OS_DATA_LIST QUERY
    OS_NAME OS_RELEASE OS_VERSION OS_PLATFORM)
list(JOIN OS_DATA_LIST " -- " OS_DATA)
message(STATUS ${OS_DATA})

string(REGEX MATCH  "([0-9]+\.[0-9]+\.[0-9]+)" OSX_SDK_MINIMUM
    ${CMAKE_OSX_DEPLOYMENT_TARGET} ".0.0.0")

include(GNUInstallDirs)
include(CTest)

##### Git command to tweak the version and commit hash
include(cmake/git_version.cmake)
file(WRITE ${PROJECT_BINARY_DIR}/PKGVERSION.txt ${PROJECT_VERSION}\n)

##### Build specifications

find_program(CCACHE_EXECUTABLE ccache)
mark_as_advanced(CCACHE_EXECUTABLE)
if(CCACHE_EXECUTABLE)
  if(NOT DEFINED CMAKE_CXX_COMPILER_LAUNCHER AND NOT
                 CMAKE_CXX_COMPILER MATCHES ".*/ccache")
    message(STATUS "Enabling ccache")
    set(CMAKE_CXX_COMPILER_LAUNCHER ${CCACHE_EXECUTABLE} CACHE STRING "")
  endif()
endif()

set(ADDRESS_SANITIZER OFF CACHE BOOL "Enable Address sanitizing")
set(APPSTORE_COMPLIANT OFF CACHE BOOL "Create an Appstore compliant, sandboxed app")
set(PROVISIONPROFILE "" CACHE STRING "Location of the embedded.provisionprofile")
if (APPSTORE_COMPLIANT)
  set(APPLE_DEVELOPER_CERT "3rd Party Mac Developer Application")
  set(APPSTORE -appstore-compliant)
else()
  set(APPLE_DEVELOPER_CERT "Developer ID Application")
endif()
if(APPLE)
  execute_process(COMMAND security find-identity -v
                  ERROR_QUIET OUTPUT_VARIABLE IDCHECK)
  string(FIND "${IDCHECK}" "${APPLE_DEVELOPER_CERT}" matchres)
  if(matchres EQUAL -1)
    unset(APPLE_DEVELOPER_CERT)
  endif()
endif()

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC_SEARCH_PATHS "${PROJECT_SOURCE_DIR}/ui")
set(CMAKE_AUTOUIC ON)
add_compile_definitions(OPENSSL_SUPPRESS_DEPRECATED QT_MESSAGELOGCONTEXT)

add_compile_options(-Wall -Wextra -Wno-unknown-pragmas)
if (ADDRESS_SANITIZER)
  add_compile_options(-fsanitize=address -O1 -fno-omit-frame-pointer -g)
  set(ASAN_LIB -fsanitize=address)
endif()
if (CMAKE_CXX_COMPILER_ID MATCHES Clang|GNU)
  add_compile_options(-fmacro-prefix-map=${PROJECT_SOURCE_DIR}=.)
endif()

file(GLOB UI_FILES ${CMAKE_AUTOUIC_SEARCH_PATHS}/*.ui)

##### Libraries and executables
add_library(xcalib STATIC)
add_executable(${CMAKE_PROJECT_NAME} img/imgres.qrc lib/main.cpp
        ${UI_FILES} ${PROJECT_BINARY_DIR}/local.h)
add_executable(xcadoc ${PROJECT_BINARY_DIR}/local.h)
add_executable(xcaexport ${PROJECT_BINARY_DIR}/local.h)

include_directories(${PROJECT_BINARY_DIR} ${PROJECT_SOURCE_DIR})

if (NOT OPENSSL_VERSION)
  # Educated guess
  set(OPENSSL_VERSION 3.0.0)
endif()

if (APPLE)
  execute_process(COMMAND brew --prefix OUTPUT_STRIP_TRAILING_WHITESPACE
                  OUTPUT_VARIABLE BREW_PREFIX ERROR_QUIET)
  # Preset the default brew paths....
  if (NOT CMAKE_PREFIX_PATH)
    set(CMAKE_PREFIX_PATH ${BREW_PREFIX}/opt/qt@6/ ${BREW_PREFIX}/opt/openssl@3)
  endif()
elseif(WIN32)
  if (NOT CMAKE_PREFIX_PATH)
    set(CMAKE_PREFIX_PATH $ENV{UserProfile} $ENV{PROGRAMFILES}/OpenSSL-Win64)
  endif()
  if (OPENSSL_VERSION VERSION_GREATER_EQUAL 3.0.0)
    set(OPENSSL_LIBRARY_MAJOR "3")
  else()
    set(OPENSSL_LIBRARY_MAJOR "1_1")
  endif()
  file(TO_CMAKE_PATH $ENV{WINDIR}/system32 SYSTEM32)
endif()

set(QTFIXEDVERSION Qt6 Qt5 CACHE STRING "Force a specific QT version 'Qt5' or 'Qt6'")
find_package(OpenSSL REQUIRED)

##### Chinese GM/T (国密) support: SM2 / SM3 / SM4
# XCA_GM=AUTO enables it when the crypto library (Tongsuo or OpenSSL)
# provides the SM algorithms, ON requires them, OFF disables the GM edition.
set(XCA_GM AUTO CACHE STRING "Chinese GM/T SM2/SM3/SM4 support (AUTO, ON, OFF)")
set_property(CACHE XCA_GM PROPERTY STRINGS AUTO ON OFF)
include(cmake/gm_support.cmake)

# local.h needs the results of the OpenSSL/Tongsuo detection above
configure_file(local.h.in local.h)
find_package(QT NAMES ${QTFIXEDVERSION} REQUIRED COMPONENTS Core)
set(QT Qt${QT_VERSION_MAJOR})
find_package(${QT} REQUIRED COMPONENTS Core Widgets Sql Help LinguistTools)
find_package(${QT} QUIET COMPONENTS Test)

include(cmake/openssl_v_err.cmake)
if (${QT_VERSION} VERSION_LESS 5.10.0)
  message(FATAL_ERROR "Qt-${QT_VERSION} not supported, please use at least Qt-5.10.1")
endif()
message(STATUS "Using ${QT} with version ${QT_VERSION}")

get_target_property(qmake_executable ${QT}::qmake IMPORTED_LOCATION)
get_filename_component(_qt_bin_dir "${qmake_executable}" DIRECTORY)

if (NOT OPENSSL_ROOT_DIR)
  get_filename_component(OPENSSL_ROOT_DIR ${OPENSSL_INCLUDE_DIR} DIRECTORY)
endif()
file(TO_CMAKE_PATH ${OPENSSL_ROOT_DIR}  OPENSSL_ROOT_DIR)

target_link_libraries(xcalib
    OpenSSL::Crypto
    ${QT}::Widgets ${QT}::Core ${QT}::Sql ${QT}::Help
    ${ASAN_LIB}
)
target_link_libraries(xcadoc
    OpenSSL::Crypto ${QT}::Core ${ASAN_LIB}
)
target_link_libraries(xcaexport ${QT}::Core ${ASAN_LIB})
target_link_libraries(${CMAKE_PROJECT_NAME} ${ASAN_LIB} xcalib)

if (APPLE)

  find_library(IOKIT_LIBRARY IOKit)
  find_library(COREFOUNDATION_LIBRARY CoreFoundation)
  target_link_libraries(${CMAKE_PROJECT_NAME}
    ${IOKIT_LIBRARY} ${COREFOUNDATION_LIBRARY}
  )
  set(CMAKE_MACOSX_BUNDLE ON)

  # Avoid bin/ sub-directory in DMG
  set(RUNTIME_DEST .)
  set(CMAKE_CXX_VISIBILITY_PRESET hidden)

elseif(WIN32)

  set(RUNTIME_DEST ${CMAKE_PROJECT_NAME}-portable-${PROJECT_VERSION})
  if (${QT_VERSION} VERSION_LESS 6.7.0)
    set(WINDOWSSTYLE "windowsvistastyle")
  else()
    set(WINDOWSSTYLE "modernwindowsstyle")
  endif()

else()

  set(RUNTIME_DEST ${CMAKE_INSTALL_BINDIR})

endif()

macro(guid content)
  string(
    UUID GUID NAMESPACE 8c1f3b62-4a7e-4d0b-9f21-6e5b2c9a7d41
    NAME ${content} TYPE SHA1
  )
endmacro()

macro(WixFile filename content)
  file(WRITE ${filename} "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
    "<Wix xmlns=\"http://schemas.microsoft.com/wix/2006/wi\">\n<Fragment>\n"
    ${content}
    "\n</Fragment>\n</Wix>")
endmacro()

add_custom_target(tests)

##### Sub directories

add_subdirectory(lib)
add_subdirectory(widgets)

include(lang/CMakeLists.txt)
include(misc/CMakeLists.txt)
include(doc/CMakeLists.txt)
include(img/CMakeLists.txt)
include(test/CMakeLists.txt)

install(TARGETS ${CMAKE_PROJECT_NAME}
    RESOURCE DESTINATION Resources
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${RUNTIME_DEST}
)

set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_VENDOR "${XCA_GM_AUTHOR}")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "${PROJECT_DESCRIPTION}")
set(CPACK_PACKAGE_HOMEPAGE_URL "${XCA_GM_URL}")

###############################################
##### Host specific settings

if(APPLE)
  set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES
    MACOSX_BUNDLE TRUE
    XCODE_ATTRIBUTE_INSTALL_PATH /Applications
    XCODE_ATTRIBUTE_SKIP_INSTALL No
    RESOURCE "${APP_RESOURCES}"
  )
  find_program(MACDEPLOYQT_EXECUTABLE macdeployqt HINTS ${_qt_bin_dir})
  find_program(INSTALL_NAME_TOOL install_name_tool HINTS ${_qt_bin_dir})

  add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${MACDEPLOYQT_EXECUTABLE} $<TARGET_FILE_DIR:${PROJECT_NAME}>/../..
        -always-overwrite ${APPSTORE}
    COMMENT "Running macdeployqt..."
  )
  if (TONGSUO_FOUND)
    # Qt (QtNetwork) links its own OpenSSL libcrypto.3.dylib, which shares
    # the file name with Tongsuo's library. macdeployqt bundles only one of
    # them, so the GM edition would silently run with plain OpenSSL.
    # Bundle Tongsuo's libcrypto under a distinct name for the xca binary.
    get_filename_component(TONGSUO_CRYPTO_REAL "${OPENSSL_CRYPTO_LIBRARY}" REALPATH)
    # macdeployqt names the bundled copy after the library's install name
    execute_process(COMMAND otool -D "${TONGSUO_CRYPTO_REAL}"
                    OUTPUT_VARIABLE TONGSUO_CRYPTO_ID ERROR_QUIET
                    OUTPUT_STRIP_TRAILING_WHITESPACE)
    string(REGEX REPLACE ".*\n" "" TONGSUO_CRYPTO_ID "${TONGSUO_CRYPTO_ID}")
    if (NOT TONGSUO_CRYPTO_ID)
      set(TONGSUO_CRYPTO_ID "${TONGSUO_CRYPTO_REAL}")
    endif()
    get_filename_component(TONGSUO_CRYPTO_NAME "${TONGSUO_CRYPTO_ID}" NAME)
    string(REPLACE "libcrypto" "libcrypto-tongsuo" TONGSUO_BUNDLE_NAME "${TONGSUO_CRYPTO_NAME}")
    set(TONGSUO_BUNDLE_LIB "@executable_path/../Frameworks/${TONGSUO_BUNDLE_NAME}")
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy "${TONGSUO_CRYPTO_REAL}"
          "$<TARGET_FILE_DIR:${PROJECT_NAME}>/../Frameworks/${TONGSUO_BUNDLE_NAME}"
      COMMAND ${INSTALL_NAME_TOOL} -id "${TONGSUO_BUNDLE_LIB}"
          "$<TARGET_FILE_DIR:${PROJECT_NAME}>/../Frameworks/${TONGSUO_BUNDLE_NAME}"
      COMMAND ${INSTALL_NAME_TOOL}
          -change "@executable_path/../Frameworks/${TONGSUO_CRYPTO_NAME}" "${TONGSUO_BUNDLE_LIB}"
          -change "${OPENSSL_CRYPTO_LIBRARY}" "${TONGSUO_BUNDLE_LIB}"
          -change "${TONGSUO_CRYPTO_REAL}" "${TONGSUO_BUNDLE_LIB}"
          -change "${TONGSUO_CRYPTO_ID}" "${TONGSUO_BUNDLE_LIB}"
          "$<TARGET_FILE:${PROJECT_NAME}>"
      COMMAND codesign --force --sign -
          "$<TARGET_FILE_DIR:${PROJECT_NAME}>/../Frameworks/${TONGSUO_BUNDLE_NAME}"
      COMMAND codesign --force --sign - "$<TARGET_FILE:${PROJECT_NAME}>"
      COMMENT "Bundling Tongsuo ${TONGSUO_CRYPTO_NAME} as ${TONGSUO_BUNDLE_NAME}"
    )
  endif()
  foreach(installdir ${CMAKE_PREFIX_PATH})
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
      COMMAND test ! -e $<TARGET_FILE_DIR:${PROJECT_NAME}>/../PlugIns/sqldrivers/libqsqlmysql.dylib ||
          ${INSTALL_NAME_TOOL} -change ${installdir}/lib/libmariadb.3.dylib @rpath/libmariadb.3.dylib
          $<TARGET_FILE_DIR:${PROJECT_NAME}>/../PlugIns/sqldrivers/libqsqlmysql.dylib
    )
  endforeach()
  if (PROVISIONPROFILE)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
      COMMAND cmake -E copy ${PROVISIONPROFILE}
              $<TARGET_FILE_DIR:${PROJECT_NAME}>/../embedded.provisionprofile)
  endif()
  if (NOT DEFINED APPLE_DEVELOPER_CERT)
    # macdeployqt leaves some bundled dylibs (e.g. libbrotlicommon) with an
    # invalid signature; arm64 kills the process when such a library is
    # mapped. Re-sign the whole bundle ad-hoc so the build tree app runs.
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
      COMMAND codesign --force --deep --sign - $<TARGET_FILE_DIR:${PROJECT_NAME}>/../..
      COMMENT "Ad-hoc signing ${PROJECT_NAME}.app ..."
    )
  endif()
  if (DEFINED APPLE_DEVELOPER_CERT)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
      COMMAND codesign --deep --signature-size=96000
                --options=runtime --timestamp
        --entitlements ${PROJECT_SOURCE_DIR}/misc/entitlement${APPSTORE}.plist
        --force -s ${APPLE_DEVELOPER_CERT}
        $<TARGET_FILE_DIR:${PROJECT_NAME}>/../..
      COMMENT "Running codesign for ${APPLE_DEVELOPER_CERT} ..."
    )
  endif()

  if (NOT DEFINED APPLE_DEVELOPER_CERT)
    # The install step rewrites the RPATH with install_name_tool, which
    # invalidates the ad-hoc signature. Unsigned arm64 binaries are killed
    # at launch, so re-sign the installed bundle ad-hoc.
    install(CODE "execute_process(COMMAND codesign --force --deep --sign - \"\${CMAKE_INSTALL_PREFIX}/${PROJECT_NAME}.app\")")
  endif()

  set(CPACK_BUNDLE_NAME ${XCA_GM_NAME})
  set(CPACK_GENERATOR "DragNDrop") #;productbuild")

elseif(WIN32)
  find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS ${_qt_bin_dir})
  add_custom_command(TARGET ${CMAKE_PROJECT_NAME} POST_BUILD
    COMMAND ${WINDEPLOYQT_EXECUTABLE} $<TARGET_FILE:${PROJECT_NAME}>
        --dir ${CMAKE_BINARY_DIR} --compiler-runtime --no-translations
    COMMENT "Running windeployqt on ${CMAKE_PROJECT_NAME} ..."
  )
  add_custom_command(TARGET xcadoc POST_BUILD
    COMMAND ${WINDEPLOYQT_EXECUTABLE} $<TARGET_FILE:xcadoc>
        --dir ${CMAKE_BINARY_DIR} --no-translations
    COMMENT "Running windeployqt on xcadoc ..."
  )
  add_custom_command(TARGET xcaexport POST_BUILD
    COMMAND ${WINDEPLOYQT_EXECUTABLE} $<TARGET_FILE:xcaexport>
        --dir ${CMAKE_BINARY_DIR} --no-translations
    COMMENT "Running windeployqt on xcaexport ..."
  )
  set(QT_USE_QTMAIN TRUE)
  set(CPACK_GENERATOR WIX)
  set(CPACK_PACKAGE_VENDOR "${XCA_GM_AUTHOR}")
  set(CPACK_WIX_TEMPLATE "${PROJECT_SOURCE_DIR}/misc/xca.wxs")
  # Distinct from upstream XCA so both can be installed side by side
  set(CPACK_WIX_UPGRADE_GUID "b7e2a9c4-3d5f-4e81-a6c2-1f9d8e7b5a30")
  set(CPACK_WIX_UI_BANNER "${PROJECT_SOURCE_DIR}/img/banner.bmp")
  set(CPACK_WIX_UI_DIALOG "${PROJECT_SOURCE_DIR}/img/dialog.bmp")
  set(CPACK_WIX_LICENSE_RTF "${PROJECT_SOURCE_DIR}/misc/copyright.rtf")
  set(CPACK_WIX_EXTRA_SOURCES
     ${CMAKE_BINARY_DIR}/language.wxs
     ${CMAKE_BINARY_DIR}/doc/documentation.wxs
     ${CMAKE_BINARY_DIR}/misc/misc.wxs
     ${CMAKE_BINARY_DIR}/misc/dynamic.wxs
  )
  target_link_options(${CMAKE_PROJECT_NAME} PRIVATE -mwindows -MD)
  install(DIRECTORY
      ${CMAKE_BINARY_DIR}/platforms
      ${CMAKE_BINARY_DIR}/sqldrivers
      ${CMAKE_BINARY_DIR}/styles
      DESTINATION ${RUNTIME_DEST}
  )
  install(FILES ${CMAKE_BINARY_DIR}/${QT}Core.dll
        ${CMAKE_BINARY_DIR}/${QT}Gui.dll
        ${CMAKE_BINARY_DIR}/${QT}Help.dll
        ${CMAKE_BINARY_DIR}/${QT}Sql.dll
        ${CMAKE_BINARY_DIR}/${QT}Widgets.dll
        ${CMAKE_BINARY_DIR}/libgcc_s_seh-1.dll
        ${CMAKE_BINARY_DIR}/libstdc++-6.dll
        ${CMAKE_BINARY_DIR}/libwinpthread-1.dll
        ${SYSTEM32}/vcruntime140.dll
        ${OPENSSL_ROOT_DIR}/bin/libcrypto-${OPENSSL_LIBRARY_MAJOR}-x64.dll
      DESTINATION ${RUNTIME_DEST}
  )
  install(CODE "execute_process(COMMAND cmake -E chdir \${CMAKE_INSTALL_PREFIX} tar cvf ${RUNTIME_DEST}.zip --format=zip -- ${RUNTIME_DEST})")
  set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR})

else()
  if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX /usr/local CACHE PATH
        "Default installation directory" FORCE)
  endif()
  message(STATUS "PREFIX: " ${CMAKE_INSTALL_PREFIX})
  set(CPACK_SOURCE_GENERATOR "TGZ")
  set(CPACK_SOURCE_IGNORE_FILES \\.git/ build/ ".*~$")
  set(CPACK_VERBATIM_VARIABLES YES)
endif()

include(CPack)
