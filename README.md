# WebThemis

WebThemis enables web app developers to build Google Chrome-based applications with strong cryptographic services as provided by [https://www.github.com/cossacklabs/themis](Themis) cryptographic library.

WebThemis provides main Themis cryptographic services: 
* [Secure Message](https://github.com/cossacklabs/themis/wiki/3.3.1-Secure-Message): a simple encrypted messaging solution  for widest scope of applications. ECC + ECDSA / RSA + PSS + PKCS#8.
* [Secure Session](https://github.com/cossacklabs/themis/wiki/3.3.2-Secure-Session): session-oriented, forward secrecy messaging solution with better security guarantees, but more demanding infrastructure. ECDH key agreement, ECC & AES encryption.
* [Secure Cell](https://github.com/cossacklabs/themis/wiki/3.3.3-Secure-Cell): a multi-mode cryptographic container, suitable for storing anything from encrypted files to database records and format-preserved strings. Secure Cell is built around AES in GCM (Token and Seal modes) and CTR (Context imprint mode).

WebThemis, as all Themis components, is free, Apache 2 licensed open-source project.

