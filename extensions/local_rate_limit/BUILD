load("@proxy_wasm_cpp_sdk//bazel/wasm:wasm.bzl", "wasm_cc_binary")

wasm_cc_binary(
    name = "local_rate_limit.wasm",
    srcs = [
        "plugin.cc",
        "plugin.h",
        "bucket.cc",
        "bucket.h",
    ],
    deps = [
        "@com_google_absl//absl/strings",
        "@io_istio_proxy//extensions/common/wasm:json_util",
        "@proxy_wasm_cpp_sdk//:proxy_wasm_intrinsics",
    ],
)
