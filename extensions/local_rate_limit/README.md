# Local Rate Limit Wasm Extensions

> **Note**: This is a very basic local rate limit Wasm filter. For sophisticated production use case, please consider using [Envoy local rate limit filter](https://www.envoyproxy.io/docs/envoy/latest/configuration/http/http_filters/local_rate_limit_filter).

Before going through this guide, please read official Istio document about [Wasm module remote load](https://istio.io/latest/docs/ops/configuration/extensibility/wasm-module-distribution/).

## Deploy Local Rate Limit Wasm Extension

---

In the following guide we will configure Istio proxy to download and apply a local rate limit filter.

Two `EnvoyFilter` resources will be applied, to inject local rate limit filter into HTTP filter chain.
For example, [this configuration](./config/gateway-filter.yaml) injects the rate limit filter to `gateway`.

The first `EnvoyFilter` will inject an HTTP filter into gateway proxies. The second `EnvoyFilter` resource provides configuration for the filter.

After applying the filter, gateway should start enforce rate limiting.
Use `productpage` app as an example, to test that the filter works,

```console
foo@bar:~$ for i in {1..50}; do curl -I http://x.x.x.x/productpage; done
...
HTTP/1.1 200 OK
content-type: text/html; charset=utf-8
content-length: 5183
server: istio-envoy
date: Sat, 27 Feb 2021 22:12:46 GMT
x-envoy-upstream-service-time: 34
...
HTTP/1.1 429 Too Many Requests
date: Sat, 27 Feb 2021 22:12:47 GMT
server: istio-envoy
transfer-encoding: chunked
...
```

## Configuration Reference

---

The following proto message describes the schema of  auth filter configuration.

```protobuf

// PluginConfig defines local rate limit configuration.
// Specifically it defines a token bucket for shared by all Envoy workers.
message PluginConfig {
    // max tokens of the toek bucket.
    uint64 max_tokens = 1;

    // number of tokens to add at each refill.
    uint64 tokens_per_refill = 2;

    // interval in seconds to refill the token bucket.
    uint64 refill_interval_sec = 3;
}
```

## Feature Request and Customization

---

If you have any feature request or bug report, please open an issue in this repo.

It is highly recommended to customize the extension according to your needs.
Please take a look at [Wasm extension C++ development guide](../doc/write-a-wasm-extension-with-cpp.md) for more information about how to write your own extension.
