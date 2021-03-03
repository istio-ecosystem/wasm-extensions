load("@proxy_wasm_cpp_sdk//bazel/wasm:wasm.bzl", "wasm_cc_binary")

wasm_cc_binary(
    name = "grpc_logging.wasm",
    srcs = [
        "plugin.cc",
        "plugin.h",
    ],
    deps = [
        ":config_cc_proto",
        ":log_cc_proto",
        "@proxy_wasm_cpp_sdk//:proxy_wasm_intrinsics_lite",
        "@com_google_absl//absl/strings",
    ],
)

cc_proto_library(
    name = "config_cc_proto",
    deps = [":config_proto"],
)

proto_library(
    name = "config_proto",
    srcs = ["config.proto"],
)

cc_proto_library(
    name = "log_cc_proto",
    deps = [":log_proto"],
)

proto_library(
    name = "log_proto",
    srcs = ["log.proto"],
    deps = [
        "@com_google_protobuf//:duration_proto",
        "@com_google_protobuf//:timestamp_proto",
    ],
)
