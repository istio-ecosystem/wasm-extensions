# Istio Rate Limit Filter

This folder contains a Rate Limit WebAssembly filter implementation, which could be loaded dynamically at Istio proxy.
Currently this filter is written and tested against 1.8 Istio proxy.

## Install

To try out rate limit filter with 1.8 Istio proxy:

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
        - name: istio.local_ratelimit
        url: https://storage.googleapis.com/wasm-test/local_ratelimit.wasm
        runtime: v8
        configuration:
   ```

