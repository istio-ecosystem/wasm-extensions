load("@proxy_wasm_cpp_sdk//bazel:defs.bzl", "proxy_wasm_cc_binary")
load("//bazel:wasm.bzl", "declare_wasm_image_targets")

proxy_wasm_cc_binary(
    name = "basic_auth.wasm",
    srcs = [
        "plugin.cc",
        "plugin.h",
        "//extensions/common/wasm:base64.h",
    ],
    deps = [
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/time",
        "//extensions/common/wasm:json_util",
    ],
)

cc_library(
    name = "basic_auth_lib",
    srcs = [
        "plugin.cc",
        "//extensions/common/wasm:base64.h",
    ],
    hdrs = [
        "plugin.h",
    ],
    copts = ["-DNULL_PLUGIN"],
    deps = [
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/time",
        "//extensions/common/wasm:json_util",
        "@proxy_wasm_cpp_host//:null_lib",
    ],
)

cc_test(
    name = "basic_auth_test",
    srcs = [
        "plugin_test.cc",
    ],
    copts = ["-DNULL_PLUGIN"],
    deps = [
        ":basic_auth_lib",
        "@com_google_googletest//:gtest",
        "@com_google_googletest//:gtest_main",
        "@proxy_wasm_cpp_host//:lib",
    ],
)

declare_wasm_image_targets(
    name = "basic_auth",
    wasm_file = ":basic_auth.wasm",
)
