# XCA-GM 国密证书管理器

[![XCA-GM (Tongsuo)](https://github.com/imraax/xca-gm/actions/workflows/tongsuo.yaml/badge.svg)](https://github.com/imraax/xca-gm/actions/workflows/tongsuo.yaml)
[![CMake (plain OpenSSL)](https://github.com/imraax/xca-gm/actions/workflows/cmake.yaml/badge.svg)](https://github.com/imraax/xca-gm/actions/workflows/cmake.yaml)
[![License: BSD-3-Clause](https://img.shields.io/badge/License-BSD_3--Clause-blue.svg)](LICENSE)

**XCA-GM** 是 [XCA](https://github.com/chris2511/xca)（X Certificate and Key management）的国密分支，
基于 [铜锁 Tongsuo](https://github.com/Tongsuo-Project/Tongsuo) 密码库实现 SM2 / SM3 / SM4 支持，
是一个图形化的 X.509 证书、证书请求、密钥和 CRL 管理工具，可以作为完整的 CA 使用。

- 项目主页：<https://github.com/imraax/xca-gm>
- 作者 / 维护：RaaX
- 当前版本：**1.0.0**（基于 XCA 2.9.0 与 Tongsuo 8.4.0）
- 许可证：BSD-3-Clause（与 XCA 相同），见 [LICENSE](LICENSE)

[English](#english) · [功能](#功能) · [安装](#安装) · [从源码构建](#从源码构建) · [国密使用说明](#国密使用说明) · [已知限制](#已知限制) · [许可证与致谢](#许可证与致谢)

## 功能

XCA 的全部功能（RSA / DSA / EC / ED25519 密钥、证书、请求、CRL、模板、PKCS#11 智能卡、
SQLite / MySQL / PostgreSQL / MSSQL 数据库、导入导出各种格式）保持不变，并新增：

| 国密能力 | 说明 |
|---------|------|
| **SM2 密钥** | GB/T 32918 固定曲线，生成、导入、导出（PEM / DER / PKCS#8） |
| **SM2-with-SM3 签名** | 证书、证书请求（CSR）、吊销列表（CRL）均可用 SM2 密钥签名 |
| **SM3 摘要** | 所有哈希选择框提供 SM3；SM2 密钥只允许 SM3 |
| **SM4 加密私钥** | 新增导出格式 “PKCS #8 SM4 加密” 与 “证书 + PKCS#8 SM4” |
| **国密 PKCS#12** | SM4-CBC (PBES2) 加密内容，HMAC-SM3 完整性校验；新建数据库默认即为 SM4-CBC |
| **中文界面** | 补全简体中文翻译；中文界面默认显示中文的 DN 字段名（国家代码、通用名称……） |

## 安装

从 [Releases](https://github.com/imraax/xca-gm/releases) 下载对应平台的安装包：

| 平台 | 文件 | 说明 |
|-----|------|-----|
| macOS (Apple Silicon) | `xca-gm-1.0.0-Darwin.dmg` | 把 `xca-gm.app` 拖到“应用程序”。安装包为 ad-hoc 签名，首次打开请在 app 上右键选“打开” |
| Windows x64 | `xca-gm-1.0.0-win64.msi` / `xca-gm-portable-1.0.0.zip` | 与原版 XCA 使用不同的安装目录与注册表项，可并存 |
| Linux | `xca-gm-1.0.0-Linux.tar.gz` 或自行构建 | 可执行文件为 `xca-gm` |

XCA-GM 与原版 XCA 使用不同的程序名（`xca-gm`）、应用标识（`io.github.imraax.xca-gm`）和用户设置目录，
两者可以在同一台机器上共存。数据库文件（`*.xdb`）格式与 XCA 兼容。

## 从源码构建

### 依赖

- C++17 编译器、[CMake](https://cmake.org) ≥ 3.13、Ninja 或 Make
- [Qt](https://www.qt.io) 5.10 以上或 Qt 6（Core、Widgets、Sql、Help、LinguistTools）
- [Tongsuo](https://github.com/Tongsuo-Project/Tongsuo) 8.4.0 以上（推荐），或 OpenSSL ≥ 3.0（内置 SM2/SM3/SM4，
  但缺少铜锁的国密扩展）
- 可选：[Sphinx](https://www.sphinx-doc.org) 用于生成帮助文档

### 1. 构建 Tongsuo

```sh
git clone -b 8.4.0 https://github.com/Tongsuo-Project/Tongsuo
cd Tongsuo
./config --prefix=/opt/tongsuo --libdir=lib enable-ntls no-tests
make -j$(nproc) && make install_sw
```

### 2. 构建 XCA-GM

```sh
git clone https://github.com/imraax/xca-gm
cmake -B build -DXCA_GM=ON \
      -DOPENSSL_ROOT_DIR=/opt/tongsuo -DCMAKE_PREFIX_PATH=/opt/tongsuo xca-gm
cmake --build build -j$(nproc)
ctest --test-dir build          # 运行测试（含国密端到端用例）
cd build && cpack               # 生成安装包
```

CMake 选项 `XCA_GM`：`AUTO`（默认，密码库支持 SM 算法时启用）、`ON`（必须支持，否则报错）、`OFF`（禁用国密功能）。
配置成功时会输出 `Found Tongsuo 8.4.0` 与 `GM/T SM2/SM3/SM4 support enabled`。

平台补充说明：

- **macOS**：`brew install qt cmake ninja`，配置时加上 Qt 前缀，例如
  `-DCMAKE_PREFIX_PATH="/opt/homebrew/opt/qt;/opt/tongsuo"`。Qt 自带的 OpenSSL `libcrypto.3.dylib`
  与 Tongsuo 同名，构建脚本会把 Tongsuo 的库以 `libcrypto-tongsuo.3.dylib` 打入 app bundle。
- **Linux（Debian/Ubuntu）**：`apt install build-essential cmake ninja-build qt6-base-dev qt6-tools-dev`
  （或 Qt5：`qtbase5-dev qttools5-dev libqt5sql5 libqt5help5`）。
- **Windows**：使用 MinGW 工具链，Tongsuo 需按其文档在 MSYS2 下构建，然后
  `cmake -B build -G "MinGW Makefiles" -DOPENSSL_ROOT_DIR=<tongsuo 前缀> xca-gm`。
- **Docker**：`docker build -t xca-gm .` 会在容器内自动构建 Tongsuo 与 XCA-GM，见 [INSTALL.docker](INSTALL.docker)。

## 国密使用说明

1. **新建密钥**：类型选择 **SM2**（无长度、曲线选项）。
2. **签发证书 / CSR / CRL**：选择 SM2 密钥后，签名算法只剩 **SM3**，生成的签名算法显示为 `SM2-with-SM3`。
3. **导出私钥**：选择 “PKCS #8 SM4 加密”。
4. **导出 PKCS#12**：`文件 › 选项` 中 PKCS#12 加密算法默认为 **SM4-CBC**，生成的文件可用
   `tongsuo pkcs12 -info -in file.p12` 查看，内容为 SM4-CBC (PBES2)，MAC 为 SM3。
5. `帮助 › 关于` 显示 XCA-GM 版本与所链接的 Tongsuo 版本。

更详细的说明见帮助文档 [doc/rst/gm.rst](doc/rst/gm.rst)。

## 已知限制

- 不支持 PKCS#11 令牌（智能卡）上的 SM2 密钥。
- Tongsuo 不提供 RIPEMD160 和 RC2 算法，基于 Tongsuo 构建时不会列出它们。
- PKCS#12 的 PBKDF2 PRF 仍为 hmacWithSHA256（OpenSSL `PKCS12_create` 不可配置），MAC 使用 SM3。
- SM2 密钥没有已注册的 JWK 曲线名，因此不支持 JWK 导出。

## 许可证与致谢

- XCA-GM 采用 **BSD-3-Clause** 许可证，Copyright (C) 2026 RaaX。
- XCA-GM 是 [XCA](https://github.com/chris2511/xca) 的衍生作品，
  Copyright (C) 2001 - 2024 Christian Hohnstädt，BSD-3-Clause。感谢 Christian Hohnstädt 与 XCA 的所有贡献者和翻译者。
- 链接的 [Tongsuo](https://github.com/Tongsuo-Project/Tongsuo) 采用 Apache License 2.0（见
  [misc/LICENSE-Tongsuo.txt](misc/LICENSE-Tongsuo.txt)），其中包含 OpenSSL Project 开发的软件。
- [Qt](https://www.qt.io) 采用 LGPL v3。

完整声明见 [COPYRIGHT](COPYRIGHT)。问题反馈与贡献请使用 [GitHub Issues](https://github.com/imraax/xca-gm/issues)
和 Pull Request。

---

## English

**XCA-GM** is a fork of [XCA](https://github.com/chris2511/xca) that adds the Chinese commercial
cryptography algorithms (GM/T: **SM2**, **SM3**, **SM4**) by building on the
[Tongsuo](https://github.com/Tongsuo-Project/Tongsuo) crypto library. It is a graphical tool for
creating and managing X.509 certificates, certificate requests, private keys and CRLs and can act as a full CA.

Version 1.0.0 is based on XCA 2.9.0 and Tongsuo 8.4.0. Author and maintainer: RaaX.
Project page: <https://github.com/imraax/xca-gm>. License: BSD-3-Clause (same as XCA).

Added on top of XCA:

- SM2 key pairs (GB/T 32918): generation, import and export (PEM / DER / PKCS#8)
- SM2-with-SM3 signatures for certificates, certificate requests and CRLs; SM3 in all hash selectors
- PKCS#8 private keys encrypted with SM4-CBC
- PKCS#12 files protected with SM4-CBC (PBES2) and an HMAC-SM3 integrity MAC (the default)
- Complete Simplified Chinese translation, translated DN terms by default for CJK user interfaces

XCA-GM installs side by side with XCA: the binary is `xca-gm`, the application id is
`io.github.imraax.xca-gm`, the Windows installer uses its own upgrade code and install directory.
Database files (`*.xdb`) stay compatible with XCA.

Build: install Qt and Tongsuo (see the Chinese section above for the commands), then

```sh
cmake -B build -DXCA_GM=ON -DOPENSSL_ROOT_DIR=/opt/tongsuo -DCMAKE_PREFIX_PATH=/opt/tongsuo xca-gm
cmake --build build -j$(nproc)
ctest --test-dir build
```

The CMake option `XCA_GM` accepts `AUTO` (default), `ON` and `OFF`. Limitations: no SM2 on PKCS#11 tokens,
no RIPEMD160/RC2 with Tongsuo, PKCS#12 PBKDF2 PRF stays hmacWithSHA256, no JWK export for SM2 keys.

XCA-GM is a derivative work of XCA, Copyright (C) 2001 - 2024 Christian Hohnstädt (BSD-3-Clause).
Tongsuo is licensed under the Apache License 2.0 (see `misc/LICENSE-Tongsuo.txt`) and includes software
developed by the OpenSSL Project. Qt is licensed under the LGPL v3. See [COPYRIGHT](COPYRIGHT).
