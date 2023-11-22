load("@proxy_wasm_cpp_sdk//bazel:defs.bzl", "proxy_wasm_cc_binary")
load("//bazel:wasm.bzl", "declare_wasm_image_targets")

proxy_wasm_cc_binary(
    name = "open_policy_agent.wasm",
    srcs = [
        "cache.cc",
        "cache.h",
        "plugin.cc",
        "plugin.h",
    ],
    deps = [
        "//extensions/common/wasm:json_util",
    ],
)

declare_wasm_image_targets(
    name = "open_policy_agent",
    wasm_file = ":open_policy_agent.wasm",
)
