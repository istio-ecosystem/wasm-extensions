# Basic Auth Filter User Guide

> **Note**: This is an experimental feature.
> **Note**: Basic Auth is not recommended for production usage since it is not secure enough. Please consider using Istio mTLS instead in production.

Basic Auth filter is shipped as a WebAssembly module from this repo.
It is versioned following Istio minor release (e.g. basic auth Wasm module with version 1.9.x should work with any Istio 1.9 patch versions).
All released versions could be found [here](https://github.com/istio-ecosystem/wasm-extensions/releases).

Before going through this guide, please read official Istio document about [Wasm module remote load](https://istio.io/latest/docs/ops/configuration/extensibility/wasm-module-distribution/).

## Deploy basic auth filter

---

In the following guide we will configure Istio proxy to download and apply Basic Auth filter.

Two `EnvoyFilter` resources will be applied, to inject basic auth filter into HTTP filter chain.
For example, [this configuration](./config/gateway-filter.yaml) injects the basic auth filter to `gateway`.

The first `EnvoyFilter` will inject an HTTP filter into gateway proxies. The second `EnvoyFilter` resource provides configuration for the filter.

After applying the filter, gateway should start enforce the basic auth rule.
Use `productpage` app as an example, to test that the rule works, you can curl with and without the authorization header.
For example

```console
foo@bar:~$ curl -i <GATEWAY_URL>/productpage
HTTP/1.1 401 Unauthorized
date: Wed, 09 Dec 2020 18:06:21 GMT
server: istio-envoy
content-length: 0
foo@bar:~$ curl -i -H "authorization: Basic YWRtaW4yOmFkbWluMg==" <GATEWAY_URL>/productpage
HTTP/1.1 200 OK
content-type: text/html; charset=utf-8
content-length: 4063
server: istio-envoy
date: Wed, 09 Dec 2020 18:07:19 GMT
x-envoy-upstream-service-time: 85
```

## Configuration Reference

---

The following proto message describes the schema of basic auth filter configuration.

```protobuf
message PluginConfig {
  // Specifies a list of basic auth rules
  repeated BasicAuth basic_auth_rules = 1;

  // Protection space of basic auth: https://tools.ietf.org/html/rfc7617#section-2.
  // If not provided, the default value is `istio`.
  string realm = 2;
}

// BasicAuth defines restriction rules based on three elements.
message BasicAuth {
  // HTTP path to restrict access according to match pattern specification.
  oneof match_pattern {
    // match exact pattern in request_path
    string exact = 1;

    // match prefix pattern in request_path
    string prefix = 2;

    // match suffix pattern in request_path
    string suffix = 3;
  }

  // HTTP request method operations such as GET, POST, HEAD, PUT, and DELETE.
  repeated string request_methods = 4;

  // Credentials provided in the form username:password that have access.
  // Credential could be provided in two formats: `USERNAME:PASSWD` and base64 encoded credentials.
  repeated string credentials = 5;
}
```

## Feature Request and Customization

---

If you have any feature request or bug report, please open an issue in this repo. Currently it is on the roadmap to:

* [ ] Read secret from local file. This is pending on [proxy wasm host implementation to support file read](https://github.com/proxy-wasm/proxy-wasm-cpp-host/issues/127).
* [ ] Add regex to path matching

It is recommended to customize the extension according to your needs.
Please take a look at [Wasm extension C++ development guide](../doc/write-a-wasm-extension-with-cpp.md) for more information about how to write your own extension.
