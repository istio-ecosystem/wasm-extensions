workspace(name = "istio_ecosystem_wasm_extensions")

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

PROXY_WASM_CPP_SDK_SHA = "fd0be8405db25de0264bdb78fae3a82668c03782"
PROXY_WASM_CPP_SDK_SHA256 = "c57de2425b5c61d7f630c5061e319b4557ae1f1c7526e5a51c33dc1299471b08"

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
IO_ISTIO_PROXY_SHA = "494a674e70543a319ad4865482c125581f5746bf"
IO_ISTIO_PROXY_SHA256 = "daa108440afe8107fb7da93f5da77340f51ddc850f1393b44e83c51f2ff53081"

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
