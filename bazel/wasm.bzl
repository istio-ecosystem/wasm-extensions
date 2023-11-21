load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive", "http_file")
load("@rules_pkg//pkg:tar.bzl", "pkg_tar")
load("@rules_oci//oci:defs.bzl", "oci_image", "oci_push")

def wasm_dependencies():
    FLAT_BUFFERS_VERSION = "23.3.3"

    http_archive(
        name = "com_github_google_flatbuffers",
        sha256 = "8aff985da30aaab37edf8e5b02fda33ed4cbdd962699a8e2af98fdef306f4e4d",
        strip_prefix = "flatbuffers-" + FLAT_BUFFERS_VERSION,
        url = "https://github.com/google/flatbuffers/archive/v" + FLAT_BUFFERS_VERSION + ".tar.gz",
    )

    http_file(
        name = "com_github_nlohmann_json_single_header",
        sha256 = "3b5d2b8f8282b80557091514d8ab97e27f9574336c804ee666fda673a9b59926",
        urls = [
            "https://github.com/nlohmann/json/releases/download/v3.7.3/json.hpp",
        ],
    )

def wasm_libraries():
    """
    Loads the necessary libraries for WebAssembly modules.
    """
    ABSL_VERSION = "c8b33b0191a2db8364cacf94b267ea8a3f20ad83"
    http_archive(
        name = "com_google_absl",
        sha256 = "a7803eac00bf68eae1a84ee3b9fcf0c1173e8d9b89b2cee92c7b487ea65be2a9",
        strip_prefix = "abseil-cpp-" + ABSL_VERSION,
        url = "https://github.com/abseil/abseil-cpp/archive/" + ABSL_VERSION + ".tar.gz",
    )

    # import json, base64, and flatbuffer library from istio proxy repo
    wasm_dependencies()
    
    # import google test and cpp host for unit testing
    GOOGLE_TEST_VERSION = "f8d7d77c06936315286eb55f8de22cd23c188571"
    http_archive(
        name = "com_google_googletest",
        sha256 = "7ff5db23de232a39cbb5c9f5143c355885e30ac596161a6b9fc50c4538bfbf01",
        strip_prefix = "googletest-" + GOOGLE_TEST_VERSION,
        urls = ["https://github.com/google/googletest/archive/" + GOOGLE_TEST_VERSION + ".tar.gz"],
    )

    PROXY_WASM_CPP_HOST_SHA = "f38347360feaaf5b2a733f219c4d8c9660d626f0"
    PROXY_WASM_CPP_HOST_SHA256 = "bf10de946eb5785813895c2bf16504afc0cd590b9655d9ee52fb1074d0825ea3"

    http_archive(
        name = "proxy_wasm_cpp_host",
        sha256 = PROXY_WASM_CPP_HOST_SHA256,
        strip_prefix = "proxy-wasm-cpp-host-" + PROXY_WASM_CPP_HOST_SHA,
        url = "https://github.com/proxy-wasm/proxy-wasm-cpp-host/archive/" + PROXY_WASM_CPP_HOST_SHA +".tar.gz",
    )

def declare_wasm_image_targets(name, wasm_file):
    pkg_tar(
        name = "wasm_tar",
        srcs = [wasm_file],
        package_dir = "./plugin.wasm",
    )
    oci_image(
        name = "wasm_image",
        architecture = "amd64",
        os = "linux",
        tars = [":wasm_tar"],
    )
    oci_push(
        name = "push_wasm_image",
        image = ":wasm_image",
        repository = "ghcr.io/istio-ecosystem/wasm-extensions/"+name,
        remote_tags = ["$(WASM_IMAGE_TAG)"],
    )
