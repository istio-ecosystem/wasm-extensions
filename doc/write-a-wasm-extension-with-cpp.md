# Develop a Wasm extension with C++

This guide will walk you through how to write, test, deploy, and maintain a HTTP Wasm extension with C++. The code for this guide can be found under [`example` folder](../example). Before walking through this guide, please go through this [guide](./development-setup.md) for development environment set up.

## Step 1: Initialize a Bazel workspace

Create a folder for the extension code. Under the folder, craete a `WORKSPACE` file , which pulls in proxy wasm cpp SDK and necessary toolchain dependencies to build a Wasm filter. The follow is a minimum `WORKSPACE` file used by the example Wasm extension, which pulls the C++ proxy Wasm SDK and invoke several rules to import tool chain:

```python
workspace(name = "example_extension")

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

# Pulls proxy wasm cpp SDK with a specific SHA
http_archive(
    name = "proxy_wasm_cpp_sdk",
    strip_prefix = "proxy-wasm-cpp-sdk-2ccab8dfc0b435f3d7641cef7683c7861e23d179",
    url = "https://github.com/proxy-wasm/proxy-wasm-cpp-sdk/archive/2ccab8dfc0b435f3d7641cef7683c7861e23d179.tar.gz",
)

load("@proxy_wasm_cpp_sdk//bazel/dep:deps.bzl", "wasm_dependencies")

wasm_dependencies()

load("@proxy_wasm_cpp_sdk//bazel/dep:deps_extra.bzl", "wasm_dependencies_extra")

wasm_dependencies_extra()
```

You can rename the workspace to anything relevent to your extension. Other dependencies could also be imported as needed, such as JSON library, base64 library, etc. See the top level `WORKSPACE` file for examples.

## Step 2: Set up extension scaffold

Copy the [scaffold plugin code](../extensions/scaffold), which includes a plugin header file `plugin.h`, a source file `plugin.cc`, and a Bazel BUILD file `BUILD`. Give the Wasm binary a meaninful name. In the example folder, we rename it to `example.wasm`.

The plugin source code implements the follows:
* A root context, which has the same lifetime as the WebAssembly VM, handles plugin configuration, and acts as the context for any interactions with Envoy host which outlives individual streams, e.g. timer tick, HTTP or gRPC call out.
* A stream context, which has the same lifetime as each request stream and acts as the target for callbacks from each stream.

After these files are copied, try build your Wasm extension with:
```
bazel build //:example.wasm
```

After the build finishes, under the generated `bazel-bin/` folder, you should be able to find a Wasm file named as `example.wasm`. Congratulations! you just successfully built an Envoy Wasm extension!

## Step 3: Fill in extension logic

The current Wasm extension is a no-op extension, callbacks need to be filled in order to make it actually function. (TODO: add doc into proxy wasm cpp sdk about available callbacks). In this example plugin, `OnResponseHeaders` is implemented to inject a response header. Specifically, followings are added into header and source file:

`plugin.h`
```cpp
class PluginContext : public Context {
public:
 ...
 FilterHeadersStatus onResponseHeaders(uint32_t, bool) override;
 ...
};
```

`plugin.cc`
```cpp
FilterHeadersStatus PluginContext::onResponseHeaders(uint32_t, bool) {
 addResponseHeader("X-Wasm-custom", "foo");
 return FilterHeadersStatus::Continue;
}
```

After adding the method, build the extension again with the bazel command mentioned in step 2 and the `example.wasm` file should be updated now with the new logic.

## Step 4: Write integration test

To enable quick iteration and deploy your extension with confidence, it is **highly** recommended to write integration tests with your extension and the same Envoy binary Istio proxy runs. To achieve this, the same Golang integration test framework for Telemetry v2 filter could be used, which at high level does the follows:
* Downloads Envoy binary, which is built by Istio proxy postsubmit and used in istio proxy docker container.
* Spins up Envoy processes locally with customizable bootstrap template.
* Spins up a xDS server locally, which serves customizable xDS resources. In the test logic, the xDS resource will reference the local WebAssembly extension files built at former steps.
* Executes test logic which sends request through Envoy, then examines extension logic accordingly.
	
An [integration test](../example/test) is also added to the example extension, which starts up a Envoy process, configures it with a Listener which points to local example Wasm file, sends a HTTP GET request, and verifies that the response header has the desired header. To learn more about how to write an integration test, please refer to this [guide](./write-integration-test.md).

## Step 5: Write unit test

To further harden your extension, it is also **highly** recommended to write unit tests for it. In the unit test, a mock host will be created, which could implement any relevant Wasm callbacks with desired simulation. Specifically, the unit test set up will utilize `null plugin` mode provided by [C++ Wasm host](https://github.com/proxy-wasm/proxy-wasm-cpp-host), under which the extension will be built to a native binary with common C++ toolchain, instead WebAssembly binary with Wasm toolchain.

An unit test example could be found under the [basic auth plugin](../extensions/basic_auth/plugin_test.cc), where several callbacks are mocked, such as `getBuffer` is mocked to return desired configuration, `getHeaderMapValue` is mocked to return wanted header pair. The whole test needs to be wrapped with `null_plugin` namespace. To learn more about how to write unit tests for the extension, please refer to this [guide](./write-unit-test.md).

## Step 6: Push and deploy the extension

After the extension has been verified and tested, it is time to deploy the extension with Istio! Assume you have already deployed a [httpbin example app](https://github.com/istio/istio/tree/master/samples/httpbin) in your cluster, and the extension has been pushed to a blob serving service, applying the following EnvoyFilter to inject the example extension:

Apply the following [EnvoyFilter](../example/config/storage-cluster.yaml) to register the blob service cluster for Extension downloading. Here we use Google Cloud Storage as an example:
```yaml
apiVersion: networking.istio.io/v1alpha3
kind: EnvoyFilter
metadata:
  name: example-cluster
spec:
  configPatches:
  - applyTo: CLUSTER
    match:
      context: SIDECAR_INBOUND
    patch:
      operation: ADD
      value: # cluster specification
        name: google-storage
        dns_lookup_family: V4_ONLY
        type: STRICT_DNS
        connect_timeout: 5s
        transport_socket:
          name: envoy.transport_sockets.tls
        load_assignment:
          cluster_name: storage.googleapis.com
          endpoints:
          - lb_endpoints:
            - endpoint:
                address:
                  socket_address:
                    address: storage.googleapis.com
                    port_value: 443
```

Then apply the following [EnvoyFilter](../example/config/example-filter.yaml) to inject the extension into inbound sidecar listeners:
```yaml
apiVersion: networking.istio.io/v1alpha3
kind: EnvoyFilter
metadata:
  name: example-filter
spec:
  configPatches:
  - applyTo: HTTP_FILTER
    match:
      context: SIDECAR_INBOUND
      listener:
        filterChain:
          filter:
            name: envoy.http_connection_manager
            subFilter:
              name: envoy.router
    patch:
      operation: INSERT_BEFORE
      value:
        name: example-filter
        typed_config:
          '@type': type.googleapis.com/udpa.type.v1.TypedStruct
          type_url: type.googleapis.com/envoy.extensions.filters.http.wasm.v3.Wasm
          value:
            config:
              vm_config:
                code:
                  remote:
                    http_uri:
                      uri: https://storage.googleapis.com/istio-ecosystem/wasm-extensions/example.wasm
                      cluster: google-storage
                      timeout: 10s
                    retry_policy:
                      num_retries: 5
                      retry_back_off:
                        base_interval: 1s
                        max_interval: 3s
                    sha256: a96aacb4fa8fd81bb7a7112b1d46432ec1463b8142d320bc269533832aa3d9a7
                runtime: envoy.wasm.runtime.v8
```

Now curl the `httpbin` service, and `x-custom-foo` should be added into the response!
```
$ curl -I 34.123.215.139
HTTP/1.1 200 OK
server: istio-envoy
...
x-wasm-custom: foo
```

## Step 7: Maintain your extension along with Istio releases

After writing, testing, and deploying the extension successfully, it is time to make sure that your extension could evolve along with Istio. Currently it is recommended to build and deploy your extension following Istio releases. Specifically:
* Import all toolchain and deps to import with the same SHA as Istio releases.
* Cut release branch as Istio, and run integration test with proxy of the same version before deploying it.
* Version your Wasm extension module files as well as the `EnvoyFilter` configuration according to Istio version. During Istio upgrade, it will be necessary to have two `EnvoyFilters` simultaneously availble in the cluster.
