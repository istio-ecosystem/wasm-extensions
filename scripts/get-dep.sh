#!/bin/bash

set -e

# Prints proxy-wasm-cpp-sdk, proxy-wasm-cpp-host, and istio-proxy SHA based on the given branch
function usage() {
  echo "$0
    -r the release branch that this script should look at, e.g. 1.8, master."
  exit 1
}

RELEASE=""

while getopts r: arg ; do
  case "${arg}" in
    r) RELEASE="${OPTARG}";;
    *) usage;;
  esac
done

if [[ ${RELEASE} == "" ]]; then
    echo "release branch has to be provided. e.g. get-dep.sh -r 1.8"
    exit 1
fi

echo "Fetching deps SHA..."

ENVOY_TMP_DIR=$(mktemp -d -t envoy-XXXXXXXXXX)
trap "rm -rf ${ENVOY_TMP_DIR}" EXIT

pushd ${ENVOY_TMP_DIR} > /dev/null
if [[ ${RELEASE} == "master" ]]; then
  git clone --depth=1 --branch master https://github.com/envoyproxy/envoy 2> /dev/null
else
  git clone --depth=1 --branch release-${RELEASE} https://github.com/istio/envoy 2> /dev/null
fi
cd envoy

NEW_SDK_SHA=$(bazel query //external:proxy_wasm_cpp_sdk --output=build 2> /dev/null | grep -Pom1 "https://github.com/proxy-wasm/proxy-wasm-cpp-sdk/archive/\K[a-zA-Z0-9]{40}")
NEW_SDK_SHA256=$(bazel query //external:proxy_wasm_cpp_sdk --output=build 2> /dev/null | grep -Pom1 "sha256 = \"\K[a-zA-Z0-9]{64}")
NEW_HOST_SHA=$(bazel query //external:proxy_wasm_cpp_host --output=build 2> /dev/null | grep -Pom1 "https://github.com/proxy-wasm/proxy-wasm-cpp-host/archive/\K[a-zA-Z0-9]{40}")
NEW_HOST_SHA256=$(bazel query //external:proxy_wasm_cpp_host --output=build 2> /dev/null | grep -Pom1 "sha256 = \"\K[a-zA-Z0-9]{64}")

popd > /dev/null

# Istio proxy SHA update
ISTIO_PROXY_TMP_DIR=$(mktemp -d -t proxy-XXXXXXXXXX)
trap "rm -rf ${ISTIO_PROXY_TMP_DIR}" EXIT

pushd ${ISTIO_PROXY_TMP_DIR} > /dev/null

if [[ ${RELEASE} == "master" ]]; then
  git clone --depth=1 --branch main https://github.com/istio/proxy 2> /dev/null
else
  git clone --depth=1 --branch release-${RELEASE} https://github.com/istio/proxy 2> /dev/null
fi
cd proxy

NEW_PROXY_SHA=$(git rev-parse HEAD)
NEW_PROXY_SHA256=$(wget https://github.com/istio/proxy/archive/${NEW_PROXY_SHA}.tar.gz 2> /dev/null && sha256sum ${NEW_PROXY_SHA}.tar.gz | grep -Pom1 "\K[a-zA-Z0-9]{64}") > /dev/null

popd > /dev/null

echo "At Istio ${RELEASE}:"
echo "proxy Wasm cpp sdk SHA ${NEW_SDK_SHA} Checksum: ${NEW_SDK_SHA256}"
echo "Proxy Wasm cpp host SHA ${NEW_HOST_SHA} Checksum: ${NEW_HOST_SHA256}"
echo "Istio proxy SHA ${NEW_PROXY_SHA} Checksum: ${NEW_PROXY_SHA256}"
