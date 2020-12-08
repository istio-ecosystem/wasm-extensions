load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")
load("@io_istio_proxy//bazel:wasm.bzl", "wasm_dependencies")

def wasm_libraries():
    http_archive(
        name = "com_google_absl",
        sha256 = "ec8ef47335310cc3382bdc0d0cc1097a001e67dc83fcba807845aa5696e7e1e4",
        strip_prefix = "abseil-cpp-302b250e1d917ede77b5ff00a6fd9f28430f1563",
        url = "https://github.com/abseil/abseil-cpp/archive/302b250e1d917ede77b5ff00a6fd9f28430f1563.tar.gz",
    )

    # import json, base64, and flatbuffer library from istio proxy repo
    wasm_dependencies()

    # import google test and cpp host for unit testing
    http_archive(
        name = "com_google_googletest",
        sha256 = "9dc9157a9a1551ec7a7e43daea9a694a0bb5fb8bec81235d8a1e6ef64c716dcb",
        strip_prefix = "googletest-release-1.10.0",
        urls = ["https://github.com/google/googletest/archive/release-1.10.0.tar.gz"],
    )

    PROXY_WASM_CPP_HOST_SHA = "a044a3a5bec75ce57c12d9e2b0e95e2a14f9f944"
    PROXY_WASM_CPP_HOST_SHA256 = "619e61997682931e07e92f5b64a4268715598d3aa22a41cadeeca816103d731f"
    BORINGSSL_SHA = "2192bbc878822cf6ab5977d4257a1339453d9d39"
    BORINGSSL_SHA256 = "bb55b0ed2f0cb548b5dce6a6b8307ce37f7f748eb9f1be6bfe2d266ff2b4d52b"

    http_archive(
        name = "proxy_wasm_cpp_host",
        sha256 = PROXY_WASM_CPP_HOST_SHA256,
        strip_prefix = "proxy-wasm-cpp-host-" + PROXY_WASM_CPP_HOST_SHA,
        url = "https://github.com/proxy-wasm/proxy-wasm-cpp-host/archive/" + PROXY_WASM_CPP_HOST_SHA +".tar.gz",
    )

    # needed by proxy wasm cpp host
    http_archive(
        name = "boringssl",
        sha256 = BORINGSSL_SHA256,
        strip_prefix = "boringssl-" + BORINGSSL_SHA,
        urls = ["https://github.com/google/boringssl/archive/" + BORINGSSL_SHA + ".tar.gz"],
    )
