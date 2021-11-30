load("@proxy_wasm_cpp_sdk//bazel/wasm:wasm.bzl", "wasm_cc_binary")
load("//bazel:wasm.bzl", "declare_wasm_image_targets")

wasm_cc_binary(
    name = "scaffold.wasm",
    srcs = [
        "plugin.cc",
        "plugin.h",
    ],
    deps = [
        "@proxy_wasm_cpp_sdk//:proxy_wasm_intrinsics",
    ],
)

declare_wasm_image_targets(
    name = "scaffold",
    wasm_file = ":scaffold.wasm",
)
