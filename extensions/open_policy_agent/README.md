# Open Policy Agent (OPA) Client Filter User Guide

> **Note**: This open policy agent should only be used for demo purpose. You should use this extension by [customizing it](#feature-request-and-customization) according to your needs.

Before going through this guide, please read official Istio document about [Wasm module distribution](https://istio.io/latest/docs/ops/configuration/extensibility/wasm-module-distribution/).

## Deploy Open Policy Agent Filter

---

In the following guide we will configure Istio proxy to download and apply an OPA filter.

Before applying the filter, you can deploy an OPA service in `istio-system` namespace with [this configuration](../config/opa-service.yaml),
which consumes the following OPA rule:

```
default allow = false

allow = true {
    input.request_method == "GET"
    input.destination_workload == "istio-ingressgateway"
    input.request_url_path == "/status/200"
}

allow = true {
    input.request_method == "GET"
    input.destination_workload == "istio-ingressgateway"
    input.request_url_path == "/headers"
}
```

Two `EnvoyFilter` resources will be applied in order to inject OPA filter into HTTP filter chain.
For example, [this configuration](./config/example-filter.yaml) injects the OPA filter to `gateway`.
The OPA filter will extract information from a request stream, send a policy check request to the OPA server,
and decide whether to allow the request based on the response from OPA server.
It also has a LRU check cache built in, which will cache check result for configurable duration.
In the check request, the following information will be included:

```json
{
    "input": {
        "request_method": "GET",
        "input.source_principal": "spiffe://cluster.local/ns/default/sa/client",
        "input.destination_workload": "echo-server",
        "input.request_url_path": "/echo"
    }
}
```

The first `EnvoyFilter` will inject an HTTP filter into gateway proxies. The second `EnvoyFilter` resource provides configuration for the filter.

After applying the filter, gateway should start sending request to OPA server for policy check.
Use `httpbin` app as an example, to test that the rule works, you can curl with and without the authorization header.
For example

```console
foo@bar:~$ curl -i <GATEWAY_URL>/headers
HTTP/1.1 200 OK
...
foo@bar:~$ curl -i <GATEWAY_URL>/status
HTTP/1.1 403 Forbidden
...
foo@bar:~$ curl -i 104.155.135.213/status/200
HTTP/1.1 200 OK
...
...
```

## Configuration Reference

---

The following proto message describes the schema of OPA filter configuration.

```protobuf
message PluginConfig {
  // Host for OPA check call. This will be used as host header.
  string opa_service_host = 1;

  // Envoy cluster for OPA HTTP call.
  string opa_cluster_name = 2;

  // Cache entry valid duration in seconds.
  string cache_valid_for_sec = 3;
}
```

## Feature Request and Customization

---

It is recommended to customize the extension according to your needs.
Please take a look at [Wasm extension C++ development guide](../doc/write-a-wasm-extension-with-cpp.md) for more information about how to write your own extension.
