workspace(name = "istio_ecosystem_wasm_extensions")

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

PROXY_WASM_CPP_SDK_SHA = "258b4c6974dba5255a9c433450971a56b29228ff"
PROXY_WASM_CPP_SDK_SHA256 = "8ff6231a5f0cc07bc865293e56eb37f60f5fd8d5a3889455c4e4ad9dbe54a5f7"

http_archive(
    name = "proxy_wasm_cpp_sdk",
    sha256 = PROXY_WASM_CPP_SDK_SHA256,
    strip_prefix = "proxy-wasm-cpp-sdk-" + PROXY_WASM_CPP_SDK_SHA,
    url = "https://github.com/proxy-wasm/proxy-wasm-cpp-sdk/archive/" + PROXY_WASM_CPP_SDK_SHA + ".tar.gz",
)

load("@proxy_wasm_cpp_sdk//bazel/dep:deps.bzl", "wasm_dependencies")

wasm_dependencies()

load("@proxy_wasm_cpp_sdk//bazel/dep:deps_extra.bzl", "wasm_dependencies_extra")

wasm_dependencies_extra()

### optional imports ###
# To import commonly used libraries from istio proxy, such as base64, json, and flatbuffer.
IO_ISTIO_PROXY_SHA = "3f3a4e9ef5d9e6e44589c92c0aa8ccc00a83d691"
IO_ISTIO_PROXY_SHA256 = "b3f21eb50e1a8adba214f38fb805c13a65e4b41846f377a83b203d799cda5b04"

http_archive(
    name = "io_istio_proxy",
    sha256 = IO_ISTIO_PROXY_SHA256,
    strip_prefix = "proxy-" + IO_ISTIO_PROXY_SHA,
    url = "https://github.com/istio/proxy/archive/" + IO_ISTIO_PROXY_SHA + ".tar.gz",
)

load("@istio_ecosystem_wasm_extensions//bazel:wasm.bzl", "wasm_libraries")

wasm_libraries()

# To import proxy wasm cpp host, which will be used in unit testing.
load("@proxy_wasm_cpp_host//bazel:repositories.bzl", "proxy_wasm_cpp_host_repositories")

proxy_wasm_cpp_host_repositories()

load("@proxy_wasm_cpp_host//bazel:dependencies.bzl", "proxy_wasm_cpp_host_dependencies")

proxy_wasm_cpp_host_dependencies()
