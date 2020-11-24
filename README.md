# Istio Ecosystem Wasm Extensions
[![Test Status][test-badge]][test-link]

This repository contains several canonical Wasm extensions, which intend to demonstrate:
* Development pattern of Wasm extension.
* Best practice to test, build, and release a Wasm extension.

# Extensions

* [C++ empty scaffold](/extensions/scaffold/)
* [Zig scaffold](/extensions/zig_demo/)
* [Basic auth](/extensions/basic_auth/)

# Guides

* [Development set up](doc/development-setup.md)
* [How to write, test, and deploy, and maintain a Wasm extension with C++](./doc/write-a-wasm-extension-with-cpp.md)
* [Write integration test for your extension](./doc/write-integration-test.md)
* [Write unit test for your extension](./doc/write-unit-test.md)

[test-badge]: https://github.com/istio-ecosystem/wasm-extensions/workflows/Test/badge.svg
[test-link]: https://github.com/istio-ecosystem/wasm-extensions/actions?query=workflow%3ATest