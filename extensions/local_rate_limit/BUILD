load("@proxy_wasm_cpp_sdk//bazel:defs.bzl", "proxy_wasm_cc_binary")
load("//bazel:wasm.bzl", "declare_wasm_image_targets")

proxy_wasm_cc_binary(
    name = "local_rate_limit.wasm",
    srcs = [
        "bucket.cc",
        "bucket.h",
        "plugin.cc",
        "plugin.h",
    ],
    deps = [
        "@com_google_absl//absl/strings",
        "//extensions/common/wasm:json_util",
    ],
)

declare_wasm_image_targets(
    name = "local_rate_limit",
    wasm_file = ":local_rate_limit.wasm",
)
