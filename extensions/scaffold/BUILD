load("@proxy_wasm_cpp_sdk//bazel:defs.bzl", "proxy_wasm_cc_binary")
load("//bazel:wasm.bzl", "declare_wasm_image_targets")

proxy_wasm_cc_binary(
    name = "scaffold.wasm",
    srcs = [
        "plugin.cc",
        "plugin.h",
    ],
)

declare_wasm_image_targets(
    name = "scaffold",
    wasm_file = ":scaffold.wasm",
)
