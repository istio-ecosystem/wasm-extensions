workspace(name = "istio_ecosystem_wasm_extensions")

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive", "http_file")
load("@bazel_tools//tools/build_defs/repo:git.bzl", "git_repository")

# reference the same commit as istio release-1.7
http_archive(
    name = "proxy_wasm_cpp_sdk",
    sha256 = "06f0f386dc8111082062f01e74e0c297e4a83857585519adb8727a3e7170f3b7",
    strip_prefix = "proxy-wasm-cpp-sdk-558d45a2f496e3039e50584cf8304ae535ca73de",
    url = "https://github.com/proxy-wasm/proxy-wasm-cpp-sdk/archive/558d45a2f496e3039e50584cf8304ae535ca73de.tar.gz",
)

# reference the same commit as istio release-1.7
http_archive(
    name = "proxy_wasm_cpp_host",
    sha256 = "5103e2a42374d8b241f1b28dd84652ea137ed983208e086388ff9621d08fbaaf",
    strip_prefix = "proxy-wasm-cpp-host-51e2013584cc87e0a797444b32bca0522252178d",
    url = "https://github.com/proxy-wasm/proxy-wasm-cpp-host/archive/51e2013584cc87e0a797444b32bca0522252178d.tar.gz",
)

# point to the same emscripten version as istio release-1.7
http_archive(
    name = "emscripten_toolchain",
    build_file = "//bazel/wasm:emscripten-toolchain.BUILD",
    patch_cmds = [
        "./emsdk install 1.39.6-upstream",
        "./emsdk activate --embedded 1.39.6-upstream",
    ],
    strip_prefix = "emsdk-1.39.6",
    url = "https://github.com/emscripten-core/emsdk/archive/1.39.6.tar.gz",
)

http_archive(
    name = "com_google_absl",
    sha256 = "ec8ef47335310cc3382bdc0d0cc1097a001e67dc83fcba807845aa5696e7e1e4",
    strip_prefix = "abseil-cpp-302b250e1d917ede77b5ff00a6fd9f28430f1563",
    url = "https://github.com/abseil/abseil-cpp/archive/302b250e1d917ede77b5ff00a6fd9f28430f1563.tar.gz",
)

http_archive(
    name = "istio_proxy",
    sha256 = "bd923c4af7f9e650788ee6147c12f5145a0100360de0c7b2c91e4936ff18d489",
    strip_prefix = "proxy-cb64d6d2fc5d4eb174544867341b655b6960edd7",
    url = "https://github.com/istio/proxy/archive/cb64d6d2fc5d4eb174544867341b655b6960edd7.tar.gz",
)

http_file(
    name = "com_github_nlohmann_json_single_header",
    sha256 = "3b5d2b8f8282b80557091514d8ab97e27f9574336c804ee666fda673a9b59926",
    urls = [
        "https://github.com/nlohmann/json/releases/download/v3.7.3/json.hpp",
    ],
)

http_archive(
    name = "com_google_googletest",
    sha256 = "9dc9157a9a1551ec7a7e43daea9a694a0bb5fb8bec81235d8a1e6ef64c716dcb",
    strip_prefix = "googletest-release-1.10.0",
    urls = ["https://github.com/google/googletest/archive/release-1.10.0.tar.gz"],
)

# required by com_google_protobuf
http_archive(
    name = "bazel_skylib",
    sha256 = "97e70364e9249702246c0e9444bccdc4b847bed1eb03c5a3ece4f83dfe6abc44",
    urls = [
        "https://mirror.bazel.build/github.com/bazelbuild/bazel-skylib/releases/download/1.0.2/bazel-skylib-1.0.2.tar.gz",
        "https://github.com/bazelbuild/bazel-skylib/releases/download/1.0.2/bazel-skylib-1.0.2.tar.gz",
    ],
)

load("@bazel_skylib//:workspace.bzl", "bazel_skylib_workspace")

bazel_skylib_workspace()

git_repository(
    name = "com_google_protobuf",
    commit = "655310ca192a6e3a050e0ca0b7084a2968072260",
    remote = "https://github.com/protocolbuffers/protobuf",
)
