# Copyright Istio Authors.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

workspace(name = "istio_ecosystem_wasm_extensions")

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

http_archive(
    name = "proxy_wasm_cpp_sdk",
    sha256 = "213d0b441bcc3df2c87933b24a593b5fd482fa8f4db158b707c60005b9e70040",
    strip_prefix = "proxy_wasm_cpp_sdk-d2b3614714ae829cdbbcdb2e5fc60d0b010ec862",
    url = "https://github.com/proxy-wasm/proxy-wasm-cpp-sdk/archive/7afb39d868a973caa6216a535c24e37fb666b6f3.tar.gz",
)
