# Copyright Istio Authors. All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
################################################################################
#

licenses(["notice"])  # Apache 2

load("@proxy_wasm_cpp_sdk//bazel/wasm:wasm.bzl", "wasm_cc_binary")

wasm_cc_binary(
    name = "attributegen.wasm",
    srcs = [
        "plugin.cc",
        "plugin.h",
        "//extensions/common:context.cc",
        "//extensions/common:context.h",
        "//extensions/common:util.cc",
        "//extensions/common:util.h",
        "//extensions/common:node_info_bfbs_generated.h",
        "//extensions/common:node_info_generated.h",
    ],
    copts = ["-UNULL_PLUGIN"],
    deps = [
        ":config_cc_proto",
        "@io_istio_proxy//extensions/common/wasm:json_util",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/time",
        "@proxy_wasm_cpp_sdk//:proxy_wasm_intrinsics_full",
        "@proxy_wasm_cpp_sdk//contrib:contrib_lib",
        "@com_github_google_flatbuffers//:flatbuffers",
        "@com_github_google_flatbuffers//:runtime_cc",
    ],
)

cc_proto_library(
    name = "config_cc_proto",
    visibility = ["//visibility:public"],
    deps = ["config_proto"],
)

proto_library(
    name = "config_proto",
    srcs = ["config.proto"],
)


