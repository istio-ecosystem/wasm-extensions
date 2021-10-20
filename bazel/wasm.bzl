load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")
load("@io_istio_proxy//bazel:wasm.bzl", "wasm_dependencies")
load("@bazel_skylib//rules:copy_file.bzl", "copy_file")
load(
    "@io_bazel_rules_docker//container:container.bzl",
    "container_image",
    "container_push",
)

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

    PROXY_WASM_CPP_HOST_SHA = "03185974ef574233a5f6383311eb74a380146fe2"
    PROXY_WASM_CPP_HOST_SHA256 = "34948e3ba239cc721af8d0a0a5b678325f363cbd542bddecf2267d24780d5b4d"

    http_archive(
        name = "proxy_wasm_cpp_host",
        sha256 = PROXY_WASM_CPP_HOST_SHA256,
        strip_prefix = "proxy-wasm-cpp-host-" + PROXY_WASM_CPP_HOST_SHA,
        url = "https://github.com/proxy-wasm/proxy-wasm-cpp-host/archive/" + PROXY_WASM_CPP_HOST_SHA +".tar.gz",
    )

def declare_wasm_image_targets(name, wasm_file):
    # Rename to the spec compatible name.
    copy_file("copy_original_file", wasm_file, "plugin.wasm")
    container_image(
        name = "wasm_image",
        files = [":plugin.wasm"],
    )
    container_push(
        name = "push_wasm_image",
        format = "OCI",
        image = ":wasm_image",
        registry = "ghcr.io",
        repository = "istio-ecosystem/wasm-extensions/"+name,
        tag = "$(WASM_IMAGE_TAG)",
    )
