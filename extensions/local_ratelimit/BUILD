load("@proxy_wasm_cpp_sdk//bazel/wasm:wasm.bzl", "wasm_cc_binary")

wasm_cc_binary(
    name = "local_ratelimit.wasm",
    srcs = [
        "plugin.cc",
        "plugin.h",
    ],
    deps = [
        "//extensions/local_ratelimit/config/v1alpha1:local_ratelimit_config_cc_proto",
        "@com_google_absl//absl/container:flat_hash_map",
        "@proxy_wasm_cpp_sdk//:proxy_wasm_intrinsics",
        "@proxy_wasm_cpp_sdk//contrib:contrib_lib",
    ],
)

cc_library(
    name = "local_ratelimit_lib",
    srcs = [
        "plugin.cc",
    ],
    hdrs = [
        "plugin.h",
    ],
    copts = ["-DNULL_PLUGIN"],
    deps = [
        "//extensions/local_ratelimit/config/v1alpha1:local_ratelimit_config_cc_proto",
        "@com_google_absl//absl/container:flat_hash_map",
        "@proxy_wasm_cpp_host//:lib",
        "@proxy_wasm_cpp_sdk//contrib:contrib_lib",
    ],
)
