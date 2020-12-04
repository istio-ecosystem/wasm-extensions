workspace(name = "istio_ecosystem_wasm_extensions")

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

http_archive(
    name = "proxy_wasm_cpp_sdk",
    sha256 = "0f675ef5c4f8fdcf2fce8152868c6c6fd33251a0deb4a8fc1ef721f9ed387dbc",
    strip_prefix = "proxy-wasm-cpp-sdk-f5ecda129d1e45de36cb7898641ac225a50ce7f0",
    url = "https://github.com/proxy-wasm/proxy-wasm-cpp-sdk/archive/f5ecda129d1e45de36cb7898641ac225a50ce7f0.tar.gz",
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
