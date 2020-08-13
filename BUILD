load("//bazel/wasm:wasm.bzl", "wasm_cc_binary")

licenses(["notice"])  # Apache 2

wasm_cc_binary(
    name = "WAF_wasm.wasm",
    srcs = ["WAF_wasm.cc"],
    deps = [
        "@proxy_wasm_cpp_sdk//:proxy_wasm_intrinsics",
        "//utility:config",
        "//utility:sqli",
        "//utility:http_parser",
    ],
    linkstatic = True,
)
