# Istio Basic Auth Filter

This folder contains a Basic Authentication WebAssembly filter implementation, which could be loaded dynamically at Istio proxy.
Currently this filter is written and tested against 1.7 Istio proxy. (TODO: add postsubmit to upload basic auth wasm extensions)

## Install

To try out basic auth filter with 1.7 Istio proxy:

1) install [extension distribution server](https://github.com/istio/proxy/tree/master/tools/extensionserver#usage),
   and configure it to fetch and generate basic auth filter configuration as following:

   ```yaml
    apiVersion: v1
    kind: ConfigMap
    metadata:
    name: extensionserver
    data:
    extension.yaml: |
        extensions:
        - name: istio.basic_auth
        url: https://storage.googleapis.com/wasm-test/basic_auth.wasm
        runtime: v8
        configuration:
            basic_auth_rules:
            - prefix: /
              request_methods:
                - GET
                - POST
              credentials:
                - ok:test
                - admin:admin
                - admin2:admin2
   ```

1) Apply EnvoyFilter to inject basic auth filter into http filter chain. The filter will be configured to fetch basic auth 
   WebAssembly module from the extension server. The following example EnvoyFilter applies basic auth filter to the
   ingressgateway proxy. Note the name of the filter has to be the same as the one specified in config map.

   ```yaml
    apiVersion: networking.istio.io/v1alpha3
    kind: EnvoyFilter
    metadata:
      name: basic-auth-1.7
      namespace: istio-system
    spec:
      configPatches:
      - applyTo: HTTP_FILTER
        match:
          context: GATEWAY
          listener:
            filterChain:
              filter:
                name: envoy.http_connection_manager
          proxy:
            proxyVersion: ^1\.7.*
        patch:
          operation: INSERT_BEFORE
          value:
            name: istio.basic_auth
            config_discovery:
              config_source:  
                api_config_source:
                  api_type: GRPC
                  transport_api_version: V3
                  grpc_services:
                  - google_grpc:
                      target_uri: extensionserver.default.svc.cluster.local:8080
                      stats_uri: extensionserver
              type_urls: ["envoy.extensions.filters.http.wasm.v3.Wasm"]
   ```

## Configuration

The following proto describes the schema of basic auth filter configuration.

```protobuf
message PluginConfig {
  // Specifies a list of basic auth rules
  repeated BasicAuth basic_auth_rules = 1;
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
  repeated string credentials = 5;
}
```
