# Detect Tongsuo and the availability of the Chinese GM/T algorithms
# SM2 (signature), SM3 (digest) and SM4 (cipher) in the crypto library.
#
# Sets:
#   TONGSUO_FOUND, TONGSUO_VERSION_STRING
#   XCA_GM_ENABLED  -> adds the XCA_GM compile definition,
#                      the "-gm" version suffix and the xca-gm package name

set(XCA_GM_ENABLED OFF)
set(TONGSUO_FOUND OFF)

if (OPENSSL_INCLUDE_DIR)
  set(_opensslv_h "${OPENSSL_INCLUDE_DIR}/openssl/opensslv.h")
  set(_config_h "${OPENSSL_INCLUDE_DIR}/openssl/configuration.h")
  set(_objmac_h "${OPENSSL_INCLUDE_DIR}/openssl/obj_mac.h")

  if (EXISTS "${_opensslv_h}")
    file(READ "${_opensslv_h}" _opensslv)
    string(REGEX MATCH "TONGSUO_VERSION_MAJOR[ \t]+([0-9]+)" _ "${_opensslv}")
    if (CMAKE_MATCH_1)
      set(_ts_major ${CMAKE_MATCH_1})
      string(REGEX MATCH "TONGSUO_VERSION_MINOR[ \t]+([0-9]+)" _ "${_opensslv}")
      set(_ts_minor ${CMAKE_MATCH_1})
      string(REGEX MATCH "TONGSUO_VERSION_PATCH[ \t]+([0-9]+)" _ "${_opensslv}")
      set(_ts_patch ${CMAKE_MATCH_1})
      set(TONGSUO_FOUND ON)
      set(TONGSUO_VERSION_STRING "${_ts_major}.${_ts_minor}.${_ts_patch}")
      message(STATUS "Found Tongsuo ${TONGSUO_VERSION_STRING} (OpenSSL API ${OPENSSL_VERSION})")
    endif()
  endif()

  # The SM algorithms are part of OpenSSL >= 1.1.1 and Tongsuo unless
  # disabled with no-sm2 / no-sm3 / no-sm4 at configure time
  set(_sm_ok ON)
  if (EXISTS "${_config_h}")
    file(READ "${_config_h}" _config)
    foreach(alg SM2 SM3 SM4)
      string(REGEX MATCH "#[ \t]*define[ \t]+OPENSSL_NO_${alg}[ \t\n]" _no "${_config}")
      if (_no)
        message(STATUS "Crypto library built without ${alg}")
        set(_sm_ok OFF)
      endif()
    endforeach()
  endif()
  if (EXISTS "${_objmac_h}")
    file(READ "${_objmac_h}" _objmac)
    foreach(nid NID_sm2 NID_sm3 NID_sm4_cbc NID_SM2_with_SM3)
      string(FIND "${_objmac}" "define ${nid} " _pos)
      if (_pos EQUAL -1)
        message(STATUS "Crypto library lacks ${nid}")
        set(_sm_ok OFF)
      endif()
    endforeach()
  else()
    set(_sm_ok OFF)
  endif()

  if (XCA_GM STREQUAL "OFF")
    message(STATUS "GM/T SM2/SM3/SM4 support disabled (XCA_GM=OFF)")
  elseif (_sm_ok)
    set(XCA_GM_ENABLED ON)
  elseif (XCA_GM STREQUAL "ON")
    message(FATAL_ERROR "XCA_GM=ON but the crypto library at "
      "${OPENSSL_INCLUDE_DIR} does not provide SM2/SM3/SM4. "
      "Build against Tongsuo: https://github.com/Tongsuo-Project/Tongsuo")
  else()
    message(STATUS "GM/T SM2/SM3/SM4 support not available in the crypto library")
  endif()
endif()

if (XCA_GM_ENABLED)
  message(STATUS "Building the GM edition: xca-gm (SM2/SM3/SM4 enabled)")
  set(XCA_GM ON)
  set(XCA_VERSION_SUFFIX "-gm")
  set(CPACK_PACKAGE_NAME "${CMAKE_PROJECT_NAME}-gm")
  add_compile_definitions(XCA_GM)
else()
  unset(XCA_GM)
  set(XCA_VERSION_SUFFIX "")
endif()
