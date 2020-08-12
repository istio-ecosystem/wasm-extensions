load("@bazel_tools//tools/build_defs/repo:git.bzl", "git_repository")
load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

######################
# Proxy-Wasm C++ SDK #
######################
BUILD_ALL_CONTENT = """filegroup(name = "all", srcs = glob(["**"]), visibility = ["//visibility:public"])"""
http_archive(
    name = "emscripten_toolchain_thing",
    build_file_content = BUILD_ALL_CONTENT,
    sha256 = "4ac0f1f3de8b3f1373d435cd7e58bd94de4146e751f099732167749a229b443b",
    patch_cmds = [
        "./emsdk install 1.39.6-upstream",
        "./emsdk activate --embedded 1.39.6-upstream",
    ],
    strip_prefix = "emsdk-1.39.6",
    urls = ["https://github.com/emscripten-core/emsdk/archive/1.39.6.tar.gz"],
)

http_archive(
    name = "emscripten_toolchain",
    build_file_content = BUILD_ALL_CONTENT,
    patch_cmds = [
        "./emsdk install 1.39.19-upstream",
        "./emsdk activate --embedded 1.39.19-upstream",
    ],
    strip_prefix = "emsdk-dec8a63594753fe5f4ad3b47850bf64d66c14a4e",
    url = "https://github.com/emscripten-core/emsdk/archive/dec8a63594753fe5f4ad3b47850bf64d66c14a4e.tar.gz",
)

http_archive(
    name = "proxy_wasm_cpp_sdk_old",
    strip_prefix = "proxy-wasm-cpp-sdk-68920464571265b237e002bfd23f91d0f9328975",
    urls = ["https://github.com/proxy-wasm/proxy-wasm-cpp-sdk/archive/68920464571265b237e002bfd23f91d0f9328975.tar.gz"],
    sha256 = "3276701b049328ce11cfb87d524ec0216b80d73a99787017a75828778301f31b",
)

http_archive(
    name = "proxy_wasm_cpp_sdk",
    strip_prefix = "proxy-wasm-cpp-sdk-envoy-release-v1.15",
    urls = ["https://github.com/proxy-wasm/proxy-wasm-cpp-sdk/archive/envoy-release/v1.15.zip"],
)


#####################
# Utility Libraries #
#####################
http_archive(
    name = "github_nlohmann_json",
    urls = ["https://github.com/nlohmann/json/releases/download/v3.6.1/include.zip",],
    build_file = "nlohmann_json.BUILD",
)

http_archive(
    name = "libinjection",
    urls = ["https://github.com/client9/libinjection/archive/v3.10.0.zip",],
    sha256 = "63540b8132ae7ad351f1dd3f15fc8b1fc1ae35bfab358d328101a15173e766ff",
    build_file = "libinjection.BUILD",
    strip_prefix = "libinjection-3.10.0/src",
)

#############
# Unit Test #
#############
git_repository(
    name = "gtest",
    remote = "https://github.com/google/googletest",
    branch = "v1.10.x",
)
