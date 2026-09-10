# XCA-GM - X Certificate and Key Management with Chinese GM/T (国密) support

[![CMake](https://github.com/chris2511/xca/actions/workflows/cmake.yaml/badge.svg)](https://github.com/chris2511/xca/actions/workflows/cmake.yaml)

## __XCA-GM (国密版)__

XCA-GM is a fork of [XCA](https://github.com/chris2511/xca) that adds the
Chinese commercial cryptography algorithms (国密算法) by building against
[Tongsuo (铜锁)](https://github.com/Tongsuo-Project/Tongsuo), an OpenSSL
derivative maintained by the OpenAtom foundation:

* **SM2** key pairs (GB/T 32918): generate, import (PEM/DER/PKCS#8), export
* **SM2-with-SM3** signatures for certificates, certificate requests and CRLs
* **SM3** digest in the hash selection boxes and for fingerprints
* **SM4-CBC** encrypted PKCS#8 private keys
* **PKCS#12** files protected with SM4-CBC (PBES2) and an HMAC-SM3 integrity MAC

Everything else (RSA, DSA, EC, ED25519, PKCS#11 tokens, templates, databases)
works exactly as in upstream XCA. The GM support is enabled automatically when
the crypto library provides SM2/SM3/SM4 (`-DXCA_GM=AUTO`, the default), can be
enforced with `-DXCA_GM=ON` or disabled with `-DXCA_GM=OFF`.
The GM edition identifies itself as version `x.y.z-gm` in the *About* dialog.

XCA-GM 基于 XCA 改造，通过链接铜锁 (Tongsuo) 密码库实现国密算法支持：
SM2 密钥生成/导入/导出、SM2-with-SM3 证书/证书请求/CRL 签名、SM3 摘要、
SM4 加密的 PKCS#8 私钥以及 SM4 + HMAC-SM3 保护的 PKCS#12 文件。

### Build XCA-GM with Tongsuo

1. Build and install Tongsuo (8.4.0 or newer, any prefix works):
   ```
   git clone -b 8.4.0 https://github.com/Tongsuo-Project/Tongsuo
   cd Tongsuo
   ./config --prefix=/opt/tongsuo --libdir=lib enable-ntls no-tests
   make -j$(nproc) && make install_sw
   ```
2. Configure XCA against Tongsuo instead of the system OpenSSL:
   ```
   cmake -B build -DOPENSSL_ROOT_DIR=/opt/tongsuo -DCMAKE_PREFIX_PATH=/opt/tongsuo xca
   cmake --build build -j$(nproc)
   ```
   On macOS add the Qt prefix as well, e.g.
   `-DCMAKE_PREFIX_PATH="/opt/homebrew/opt/qt;/opt/tongsuo"`.
3. `cmake` prints `Found Tongsuo 8.4.0` and
   `Building the GM edition: xca-gm (SM2/SM3/SM4 enabled)`.
4. Run the tests with `ctest --test-dir build` (the GUI test `testxca`
   contains an SM2/SM3/SM4 end-to-end test, `test_digest` covers SM3).

A ready to use container build is provided by `Dockerfile.gm`.

Notes:
* Tongsuo drops some legacy algorithms (RIPEMD160, RC2 based PKCS#12
  encryption). They are simply not offered when building against Tongsuo.
* SM2 keys are also supported when building against plain OpenSSL >= 3.0,
  only the non-standard Tongsuo extensions (e.g. TLCP) are not needed by XCA.
* SM2 keys on PKCS#11 tokens are not supported.
* macOS: Qt links its own OpenSSL `libcrypto.3.dylib`. The app bundle therefore
  contains both libraries, Tongsuo's one is named `libcrypto-tongsuo.3.dylib`.

---


## __Release Notes__

* The latest release is *2.9.0*
* This release fixes some minor issues:
  * Improve remote database support on macosx
  * Do not revoke renewed certificate with same serial
  * Fix default template finding on linux
  * Use latest OpenSSL and Qt releases for the precompiled releases.
* Please report issues on github <https://github.com/chris2511/xca/issues>

## __Changelog:__

A detailed changelog can be found here:

<https://hohnstaedt.de/xca/index.php/software/changelog>

## __Documentation__

This application is documented in the *Help* menu and here:

<https://www.hohnstaedt.de/xca/index.php/documentation/manual>

## __Build from Source__

### Dependencies

To build XCA you need:
 - a toolchain
 - cmake: https://cmake.org
 - Qt5 or Qt6: https://www.qt.io (5.10.1 or higher)
 - OpenSSL: https://www.openssl.org (1.1.1 or higher)
   or libressl-3.6.x
 - Sphinx-Build: https://www.sphinx-doc.org

### Linux / Unix

 - Install the dependencies
   ```
   # Bookworm
   sudo apt install build-essential libssl-dev pkg-config cmake qttools5-dev python3-sphinxcontrib.qthelp
   # Bullseye
   sudo apt install build-essential libssl-dev pkg-config cmake qttools5-dev python3-sphinx
   # Either Qt5
   sudo apt install qtbase5-dev qttools5-dev-tools libqt5sql5 libqt5help5 qttools5-dev
   # Or Qt6
   sudo apt install qt6-base-dev qt6-tools-dev
   ```
 - Clone: `git clone https://github.com/chris2511/xca.git`
 - Configure: `cmake -B build xca`
 - Make: `cmake --build build -j5`
 - Install: `sudo cmake --install build`
 - Or install local and copy later as root: `DESTDIR=DEST cmake --install build --prefix /usr`

### Apple macos

- Install the dependencies
  ```
  xcode-select --install
  brew install openssl@3 qt6 python3 cmake
  pip3 install sphinx
  ```
- Clone: `git clone https://github.com/chris2511/xca.git`
- Configure: `cmake -B build xca`
- Make: `cmake --build build -j5`
- Build the DMG: `cd build && cpack`
- Build the PKG: `cd build && cpack -G productbuild`

XCA can be used with Xcode after initializing the directory with:
`cmake -G Xcode -B .`

### Windows

- Install the dependencies
  - Install Python 3.11 for windows from the store or https://www.python.org/downloads/windows/
  - Install OpenSSL from here: https://slproweb.com/download/Win64OpenSSL-3_1_5.msi and verify the sha256 from https://github.com/slproweb/opensslhashes/blob/master/win32_openssl_hashes.json
  - To install the Qt libraries, cmake and the MinGW compiler [aqtinstall](https://github.com/miurahr/aqtinstall) is used.
    Sphinx is used to generate the documentation
    ```
    pip3 install sphinx aqtinstall
    ```
  - Add the PATH shown by pip to your PATH
  - Install Qt, cmake and the MinGW toolchain
    ```
    aqt install-qt windows desktop 6.6.3 win64_mingw
    aqt install-tool windows desktop tools_mingw90 qt.tools.win64_mingw900
    aqt install-tool windows desktop tools_vcredist qt.tools.vcredist_64
    ```
  - If 7z is missing, install it from the store. `7-Zip File Manager (unofficial)` or from 7-zip.org
  - Install the "vcredist\\vcredist_64.exe"
  - Add cmake, MinGW, OpenSSL and Qt6 to your Path
    ```
    %USERPROFILE%\AppData\Local\Packages\PythonSoftwareFoundation.Python.3.10_qbz5n2kfra8p0\LocalCache\local-packages\Python310\Scripts;
    %USERPROFILE%\AppData\Local\Microsoft\WindowsApps;
    %USERPROFILE%\Tools\CMake_64\bin;
    %USERPROFILE%\Tools\mingw_64\bin;
    %USERPROFILE%\6.6.3\mingw_64\bin;
    ```
  - Create `CMAKE_PREFIX_PATH` environment variable:
    ```
    %USERPROFILE%\6.6.3\mingw_64\lib\cmake
    ```
  - Install `https://wixtoolset.org/releases/` if you want to create the MSI installer

- Clone: `git clone https://github.com/chris2511/xca.git`
- Configure: `cmake -B build -G "MinGW Makefiles" xca`
- Make: `cmake --build build -j5`
- Create the Portable App: `cmake --build build -t install`
- Build the MSI installer (and the Portable App): `cd build ; cpack`

## __SQL Remote Database Drivers__

MySQL plugins are not shipped with QT anymore because of license issues.

### Linux

- Debian: `libqt6sql6-psql` `libqt6sql6-mysql` or `libqt6sql6-odbc`.
- RPM: `libqt6-database-plugin-pgsql` `libqt6-database-plugin-mysql` `libqt6-database-plugin-odbc`

They should pull in all necessary dependencies.

### Apple macos

- **PostgreSQL**: Driver included since XCA 2.9.0
- **ODBC**: It requires the `/usr/local/opt/libiodbc/lib/libiodbc.2.dylib`.
    When installing unixodbc via `brew` the library must be symlinked from
    `/opt/homebrew/Cellar/libiodbc/3.52.16/lib/libiodbc.2.dylib`
- **MariaDB**: Driver included since XCA 2.8.0

### Windows

- **PostgreSQL**: https://www.enterprisedb.com/downloads/postgres-postgresql-downloads (Commandline tools).
  Add the `bin` directory of the Postgres installation directory to your PATH (C:\\Program Files\\PostgreSQL\\16)
- **ODBC**: Use the `ODBC Datasources 64bit app` to configure the SQL Server
- **MariaDB (MySQL)**: Install the Plugin from here: https://github.com/thecodemonkey86/qt_mysql_driver.
  Select the MinGW variant and install it as documented.
