load("@proxy_wasm_cpp_sdk//bazel/wasm:wasm.bzl", "wasm_cc_binary")

wasm_cc_binary(
    name = "open_policy_agent.wasm",
    srcs = [
        "cache.cc",
        "cache.h",
        "plugin.cc",
        "plugin.h",
    ],
    deps = [
        "@io_istio_proxy//extensions/common/wasm:json_util",
        "@proxy_wasm_cpp_sdk//:proxy_wasm_intrinsics",
    ],
)
