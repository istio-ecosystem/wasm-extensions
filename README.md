# Istio Ecosystem Wasm Extensions
[![Test Status][test-badge]][test-link]

This repository contains several canonical Wasm extensions, which intend to demonstrate:
* Development pattern of Wasm extension.
* Best practice to test, build, and release a Wasm extension.

# Extensions
* [Basic auth](/extensions/basic_auth/)
* [C++ scaffold](/extensions/scaffold/)
* [gRPC access logging](./extension/grpc_loggig)
* [Local rate limit](/extensions/local_rate_limit/)
* [Open Policy Agent (OPA) client](/extensions/opa/)
* [Zig scaffold](/extensions/zig_demo/)

# Guides

## C++
* [Development set up](doc/development-setup.md)
* [Write, test, deploy, and maintain a Wasm extension](./doc/write-a-wasm-extension-with-cpp.md)

## Test
* [Write integration test for Wasm extension](./doc/write-integration-test.md)
* [Write unit test for Wasm extension](./doc/write-cpp-unit-test.md)

[test-badge]: https://github.com/istio-ecosystem/wasm-extensions/workflows/Test/badge.svg
[test-link]: https://github.com/istio-ecosystem/wasm-extensions/actions?query=workflow%3ATest
