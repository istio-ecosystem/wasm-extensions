load("//bazel/wasm:wasm.bzl", "wasm_cc_binary")

wasm_cc_binary(
    name = "basic_auth.wasm",
    srcs = [
        "plugin.cc",
        "plugin.h",
        "@istio_proxy//extensions/common/wasm:base64.h",
    ],
    deps = [
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/time",
        "@istio_proxy//extensions/common/wasm:json_util",
        "@proxy_wasm_cpp_sdk//:proxy_wasm_intrinsics",
    ],
)

cc_library(
    name = "basic_auth",
    srcs = [
        "plugin.cc",
        "@istio_proxy//extensions/common/wasm:base64.h",
    ],
    hdrs = [
        "plugin.h",
    ],
    copts = ["-DNULL_PLUGIN"],
    deps = [
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/time",
        "@istio_proxy//extensions/common/wasm:json_util",
        "@proxy_wasm_cpp_host//:lib",
    ],
)

cc_test(
    name = "basic_auth_test",
    srcs = [
        "plugin_test.cc",
    ],
    copts = ["-DNULL_PLUGIN"],
    deps = [
        ":basic_auth",
        "@com_google_googletest//:gtest",
        "@com_google_googletest//:gtest_main",
        "@proxy_wasm_cpp_host//:lib",
    ],
)
