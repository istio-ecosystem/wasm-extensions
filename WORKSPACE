workspace(name = "istio_ecosystem_wasm_extensions")

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

http_archive(
    name = "proxy_wasm_cpp_sdk",
    sha256 = "5909993d801690ff29cc2354a36d4939d8d2b8db0cf4f331057b079b49e4863f",
    strip_prefix = "proxy-wasm-cpp-sdk-8564f6660d0fdadc55fc55a4b894bb1abd283c6c",
    url = "https://github.com/proxy-wasm/proxy-wasm-cpp-sdk/archive/8564f6660d0fdadc55fc55a4b894bb1abd283c6c.tar.gz",
)

load("@proxy_wasm_cpp_sdk//bazel/dep:deps.bzl", "wasm_dependencies")

wasm_dependencies()

load("@proxy_wasm_cpp_sdk//bazel/dep:deps_extra.bzl", "wasm_dependencies_extra")

wasm_dependencies_extra()

### optional imports ###
# To import commonly used libraries from istio proxy, such as base64, json, and flatbuffer.
http_archive(
    name = "io_istio_proxy",
    sha256 = "d41a8f33d1842cb56d9eb1f79c0a7c73abe52dac8a90bda51f365a8411495398",
    strip_prefix = "proxy-26f79858b782b1590bd482f060cda77e3b4d565c",
    url = "https://github.com/istio/proxy/archive/26f79858b782b1590bd482f060cda77e3b4d565c.tar.gz",
)

load("@istio_ecosystem_wasm_extensions//bazel:wasm.bzl", "wasm_libraries")

wasm_libraries()
