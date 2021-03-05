# Istio Ecosystem Wasm Extensions

[![Test Status][test-badge]][test-link]

This repository contains several canonical Wasm extensions, which intend to demonstrate:

* Wasm extension development pattern.
* Best practice to test, build, and release a Wasm extension.

## Extensions

* *[Basic auth](/extensions/basic_auth/)* enforces basic auth based on request host, path, and methods. From this extension, you can find how to perform local auth decision based on headers and local reply, as well as JSON configuration string parsing and base64 decoding.
  
* *[C++ scaffold](/extensions/scaffold/)* provides an empty C++ extension, which can be used as a start point to write a C++ Wasm extension.
  
* *[gRPC access logging](./extensions/grpc_logging)* makes a logging request to a gRPC service with various kinds of request and workload attributes. From this extension, you can find how to perform asynchronous telemetry reporting, fetch various request attributes and proxy properties, use protobuf and make gRPC callout.

* *JWT based routing ([WIP](https://github.com/istio-ecosystem/wasm-extensions/issues/16))* reads JWT token information from Envoy dynamic metadata written by JWT auth filter, update host header accordingly, and trigger routing recomputation. From this extension, you can find how to read dynamic metadata, manipulate headers, and affect request routing.

* *[Local rate limit](/extensions/local_rate_limit/)* applies a token bucket rate limit to incoming requests. Each request processed by the filter utilizes a single token, and if no tokens are available, the request will be denied. From this extension you can find how to share data across all plugin VMs and deny request with local reply.

* *Open Policy Agent client ([WIP](https://github.com/istio-ecosystem/wasm-extensions/pull/54))* makes HTTP callout to a Open Policy Agent (OPA) server and based on OPA server response make decision to allow or deny an incoming request. A result cache is also included to avoid expensive callout on every request. From this extension, you can find how to perform HTTP callout, and asynchronously continue or stop an incoming request based on the response of HTTP call. You will also find how to record stats, which can be scraped in the same way as Istio standard metrics.

* *[Zig scaffold](/extensions/zig_demo/)* provides an empty [Zig](https://ziglang.org/) extension, which can be used as a start point to write a Zig Wasm extension.

## Development Guides

### Write a Wasm Extension with C++

* [Development set up](doc/development-setup.md)
* [Write, test, deploy, and maintain a C++ Wasm extension](./doc/write-a-wasm-extension-with-cpp.md)
* [Write unit test with C++ Wasm extension](./doc/write-cpp-unit-test.md)

### Integration Test

* [Write integration test with Istio proxy for Wasm extension](./doc/write-integration-test.md)

[test-badge]: https://github.com/istio-ecosystem/wasm-extensions/workflows/Test/badge.svg
[test-link]: https://github.com/istio-ecosystem/wasm-extensions/actions?query=workflow%3ATest
