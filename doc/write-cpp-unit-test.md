# Wasm Extension C++ Unit Test Dev Guide

> **NOTE:** This guide requires familiarity with [GoogleTest framework](https://github.com/google/googletest). Please go through its [getting started guide](https://github.com/google/googletest#getting-started) first.

## Overview
---

In addition to [integration test](./write-integration-test.md), it is also recommended to write unit test for your extension. Unlike integration test, which executes the extension code as Wasm binary with Envoy runtime, unit test is compiled with local C++ toolchains and run natively.

In [`proxy-wasm-cpp-host`](https://github.com/proxy-wasm/proxy-wasm-cpp-host), along with `V8`, `wasmtime`, `WAVM` runtimes that execute Wasm binaries, [a `nullVM` runtime](https://github.com/proxy-wasm/proxy-wasm-cpp-host/tree/master/src/null) is also included, which runs natively compiled-in modules. The unit test will utilize `nullVm` runtime. Specifically, all host implementation of `proxy-wasm-cpp-host` (such as Envoy) needs to implements a [`Context` interface class](https://github.com/proxy-wasm/proxy-wasm-cpp-host/blob/eceb02d5b7772ec1cd78a4d35356e57d2e6d59bb/include/proxy-wasm/context.h#L128), which has many unimplemented host specific methods. In the unit test, a mock implementation of the `Context` class will be created using [GoogleTest framework](https://github.com/google/googletest), and compiled with your extension code. Within the mock implementation, we could inject desired host behavior and check if the extension interacts with host as expected.

the following guide will walk through how to write unit test based on [an example test for basic auth filter](../extensions/basic_auth/plugin_test.cc).

## Step 1: Add nullVM directives to extension code
---

The `nullVM` implementation in `proxy-wasm-cpp-host` is wrapped in `proxy_wasm::null_plugin` namespace. To make it convenient to build your extension code with `nullVM`, the extension code is also wrapped by the same namespace. Here is how to add nullVM directives to basic auth code:

`plugin.h`
```cpp
...
#ifndef NULL_PLUGIN

#include "proxy_wasm_intrinsics.h"

#else

#include "include/proxy-wasm/null_plugin.h"

namespace proxy_wasm {
namespace null_plugin {
namespace basic_auth {

#endif

...

#ifdef NULL_PLUGIN

}  // namespace basic_auth
}  // namespace null_plugin
}  // namespace proxy_wasm

#endif
```

`plugin.cc`
```cpp
...
#ifdef NULL_PLUGIN

namespace proxy_wasm {
namespace null_plugin {
namespace basic_auth {

PROXY_WASM_NULL_PLUGIN_REGISTRY

#endif

...

#ifdef NULL_PLUGIN

}  // namespace basic_auth
}  // namespace null_plugin
}  // namespace proxy_wasm

#endif
```

Then to build the extension under `nullVM` mode, the following target is added to the `BUILD` file. The target is similiar to the one generated Wasm binary, except that it uses `cc_binary` and defines `NULL_PLUGIN` macro in `copts`.

`BUILD`
```python
cc_library(
    name = "basic_auth",
    srcs = [
        "plugin.cc",
        "@io_istio_proxy//extensions/common/wasm:base64.h",
    ],
    hdrs = [
        "plugin.h",
    ],
    copts = ["-DNULL_PLUGIN"],
    deps = [
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/time",
        "@io_istio_proxy//extensions/common/wasm:json_util",
        "@proxy_wasm_cpp_host//:lib",
    ],
)
```

## Step 2: Create a scaffold for unit tests
---

Before implementing test, the scaffold needs to be created, which imports necessary library, adds the namespace wrapper, registers the extension with `nullVM` runtime, and creates a test class which initializes the `nullVM` runtime. Unlike extension code, the unit test is always compiled with nullVM mode, so there is no need to wrap the namespace with `NULL_PLUGIN` directive.

```cpp
// necessary library imports for 
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "include/proxy-wasm/context.h"
#include "include/proxy-wasm/null.h"

namespace proxy_wasm {
namespace null_plugin {
namespace basic_auth {

NullPluginRegistry* context_registry_;
RegisterNullVmPluginFactory register_basic_auth_plugin("basic_auth", []() {
  return std::make_unique<NullPlugin>(basic_auth::context_registry_);
});

class BasicAuthTest : public ::testing::Test {
 protected:
  BasicAuthTest() {
    // Initialize test VM
    test_vm_ = createNullVm();
    wasm_base_ =std::make_unique<WasmBase>(
        std::move(test_vm_), "test-vm", "", "",
        std::unordered_map<std::string, std::string>{},
        AllowedCapabilitiesMap{});
    wasm_base_->initialize("basic_auth");

    ...
  }
  ~BasicAuthTest() override {}

  std::unique_ptr<WasmBase> wasm_base_;
  std::unique_ptr<WasmVm> test_vm_;

  std::unique_ptr<PluginRootContext> root_context_;
  std::unique_ptr<PluginContext> context_;

};

}  // namespace basic_auth
}  // namespace null_plugin
}  // namespace proxy_wasm

```

The test target is also added to the `BUILD` file, which imports `:basic_auth` library built in the first step as a dependency.

```python
cc_test(
    name = "basic_auth_test",
    srcs = [
        "plugin_test.cc",
    ],
    copts = ["-DNULL_PLUGIN"],
    deps = [
        ":basic_auth",
        "@com_google_googletest//:gtest",
        "@com_google_googletest//:gtest_main",
        "@proxy_wasm_cpp_host//:lib",
    ],
)
```

## Step 3: Implement mock context
---

Next step is to implement the host mock by creating a `MockContext` class, which inherits [the base class](https://github.com/proxy-wasm/proxy-wasm-cpp-host/blob/cce535101c3b1cab61fb6bf83a61b0e9834bd957/include/proxy-wasm/context.h#L128) from `proxy-wasm-cpp-host` repo. For basic auth filter, the interactions it has with the host environment are:
* Read configuration via `getBuffer`.
* Log warning message when things go wrong via `log`.
* Get authorization headers from the host env via `getHeaderMapValue`.
* Send local access denial if the auth header fails the check.

```cpp
class MockContext : public proxy_wasm::ContextBase {
 public:
  MockContext(WasmBase* wasm) : ContextBase(wasm) {}

  MOCK_METHOD(BufferInterface*, getBuffer, (WasmBufferType));
  MOCK_METHOD(WasmResult, log, (uint32_t, std::string_view));
  MOCK_METHOD(WasmResult, getHeaderMapValue,
              (WasmHeaderMapType /* type */, std::string_view /* key */,
               std::string_view* /*result */));
  MOCK_METHOD(WasmResult, sendLocalResponse,
              (uint32_t /* response_code */, std::string_view /* body */,
               Pairs /* additional_headers */, uint32_t /* grpc_status */,
               std::string_view /* details */));
};
```

Then in the test class, inject implementation for the mock methods.

```cpp
BasicAuthTest() {
  // Initialize test VM
  ...

  // Initialize host side context
  mock_context_ = std::make_unique<MockContext>(wasm_base_.get());
  current_context_ = mock_context_.get();

  ON_CALL(*mock_context_, log(testing::_, testing::_))
      .WillByDefault([](uint32_t, std::string_view m) {
        std::cerr << m << "\n";
        return WasmResult::Ok;
      });

  ON_CALL(*mock_context_, getHeaderMapValue(WasmHeaderMapType::RequestHeaders,
                                            testing::_, testing::_))
      .WillByDefault([&](WasmHeaderMapType, std::string_view header,
                          std::string_view* result) {
        if (header == ":path") {
          *result = path_;
        }
        if (header == ":method") {
          *result = method_;
        }
        if (header == "authorization") {
          *result = authorization_header_;
        }
        return WasmResult::Ok;
      });

  // Initialize Wasm sandbox context
  root_context_ = std::make_unique<PluginRootContext>(0, "");
  context_ = std::make_unique<PluginContext>(1, root_context_.get());
}

std::unique_ptr<PluginRootContext> root_context_;
std::unique_ptr<PluginContext> context_;

std::string path_;
std::string method_;
std::string cred_;
std::string authorization_header_;
```

## Step 4: Implement test
---

Now everything is ready, it is time to implement the unit test! Let's take a look at one of the example tests. It verifies that basic auth filter `onRequestHeader` method will deny a request based on prefix match. The test implementation first makes the mocked `getBuffer` call return a desired filter configuration, and let the filter consume the configuration via `onConfigure`; then sets the desired request properties (path, method, password), and verfies the request is denied by checking that local response is sent with wanted response code.

```cpp
TEST_F(BasicAuthTest, PrefixDeny) {
  std::string configuration = R"(
{
  "basic_auth_rules": [
    { 
      "prefix": "/api",
      "request_methods":[ "GET", "POST" ],
      "credentials":[ "ok:test", "admin:admin", "admin2:admin2" ] 
    }
  ]
})";

  BufferBase buffer;
  buffer.set({configuration.data(), configuration.size()});

  EXPECT_CALL(*mock_context_, getBuffer(WasmBufferType::PluginConfiguration))
      .WillOnce([&buffer](WasmBufferType) { return &buffer; });
  EXPECT_TRUE(root_context_->onConfigure(configuration.size()));

  path_ = "/api/test";
  method_ = "GET";
  cred_ = "wrong-cred";
  authorization_header_ = "Basic " + Base64::encode(cred_.data(), cred_.size());
  EXPECT_CALL(*mock_context_, sendLocalResponse(401, testing::_, testing::_,
                                                testing::_, testing::_));
  EXPECT_EQ(context_->onRequestHeaders(0, false),
            FilterHeadersStatus::StopIteration);

  path_ = "/api/config";
  method_ = "POST";
  cred_ = "admin2:admin2";
  authorization_header_ = Base64::encode(cred_.data(), cred_.size());
  EXPECT_CALL(*mock_context_, sendLocalResponse(401, testing::_, testing::_,
                                                testing::_, testing::_));
  EXPECT_EQ(context_->onRequestHeaders(0, false),
            FilterHeadersStatus::StopIteration);
}
```
