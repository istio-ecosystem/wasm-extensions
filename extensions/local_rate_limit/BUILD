load("@proxy_wasm_cpp_sdk//bazel/wasm:wasm.bzl", "wasm_cc_binary")
load("//bazel:wasm.bzl", "declare_wasm_image_targets")

wasm_cc_binary(
    name = "local_rate_limit.wasm",
    srcs = [
        "bucket.cc",
        "bucket.h",
        "plugin.cc",
        "plugin.h",
    ],
    deps = [
        "@com_google_absl//absl/strings",
        "@io_istio_proxy//extensions/common/wasm:json_util",
        "@proxy_wasm_cpp_sdk//:proxy_wasm_intrinsics",
    ],
)

declare_wasm_image_targets(
    name = "local_rate_limit",
    wasm_file = ":local_rate_limit.wasm",
)
