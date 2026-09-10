=====================================
Chinese GM/T cryptography (XCA-GM)
=====================================

XCA-GM is the *GM edition* of XCA. It supports the Chinese commercial
cryptography algorithms (国密算法) when built against
`Tongsuo <https://github.com/Tongsuo-Project/Tongsuo>`_, an OpenSSL based
crypto library. The *About* dialog shows the version suffix ``-gm`` and the
Tongsuo version if the support is compiled in.

Algorithms
==========

SM2 keys
   The *New Key* dialog offers the key type **SM2**. SM2 keys always use the
   fixed curve of GB/T 32918, there is no key size or curve selection.
   SM2 keys can be imported from and exported to PEM, DER and PKCS#8 files.
   The traditional PEM format of SM2 keys is ``EC PRIVATE KEY``.

SM3 digest
   The hash algorithm **SM3** is offered in all hash selection boxes.
   SM2 keys must be combined with SM3, so SM3 is the only choice for
   certificates, certificate signing requests and revocation lists signed
   with SM2 keys. The signature algorithm is shown as ``SM2-with-SM3``.

SM4 cipher
   Private keys can be exported as PKCS#8 encrypted with **SM4-CBC**
   (export format *PKCS #8 SM4 encrypted*). In the *Options* dialog the
   PKCS#12 encryption algorithm **SM4-CBC** may be selected. PKCS#12 files
   are then encrypted with SM4-CBC (PBES2) and protected by an HMAC-SM3
   integrity MAC.

Limitations
===========

* SM2 keys on PKCS#11 tokens (smart cards) are not supported.
* Tongsuo does not provide the legacy algorithms RIPEMD160 and RC2.
  They are not offered when XCA is built against Tongsuo.
* JSON Web Key (JWK) export is not available for SM2 keys because
  there is no registered JWK curve name for SM2.

Building
========

Build and install Tongsuo, then point cmake to it::

   cmake -B build -DXCA_GM=ON -DOPENSSL_ROOT_DIR=/opt/tongsuo \
         -DCMAKE_PREFIX_PATH=/opt/tongsuo xca
   cmake --build build

The cmake option ``XCA_GM`` accepts ``AUTO`` (default: enable if the crypto
library provides SM2, SM3 and SM4), ``ON`` (fail if not available) and
``OFF``.
