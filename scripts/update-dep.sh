#!/bin/bash

set -eu

# update proxy-wasm-cpp-sdk, proxy-wasm-cpp-host, and istio-proxy SHA based on the given branch
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
    echo "release branch has to be provided. e.g. update-dep.sh -r 1.8"
    exit 1
fi

WD=$(pwd)

# Clone istio-envoy repo, get and update proxy Wasm sdk and host SHA
ENVOY_TMP_DIR=$(mktemp -d -t envoy-XXXXXXXXXX)
trap "rm -rf ${ENVOY_TMP_DIR}" EXIT

cd ${ENVOY_TMP_DIR}
git clone --depth=1 https://github.com/envoyproxy/envoy
cd envoy
git remote add istio-envoy https://github.com/istio/envoy
git fetch istio-envoy
cd envoy

# SDK SHA update
NEW_SDK_SHA=$(bazel query //external:proxy_wasm_cpp_sdk --output=build | grep -Pom1 "https://github.com/proxy-wasm/proxy-wasm-cpp-sdk/archive/\K[a-zA-Z0-9]{40}")
NEW_SDK_SHA256=$(bazel query //external:proxy_wasm_cpp_sdk --output=build | grep -Pom1 "sha256 = \"\K[a-zA-Z0-9]{64}")
pushd $WD
CUR_SDK_SHA=$(bazel query //external:proxy_wasm_cpp_sdk --output=build | grep -Pom1 "https://github.com/proxy-wasm/proxy-wasm-cpp-sdk/archive/\K[a-zA-Z0-9]{40}")
CUR_SDK_SHA256=$(bazel query //external:proxy_wasm_cpp_sdk --output=build | grep -Pom1 "sha256 = \"\K[a-zA-Z0-9]{64}")
sed -i "s/${CUR_SDK_SHA}/${NEW_SDK_SHA}/g" WORKSPACE
sed -i "s/${CUR_SDK_SHA256}/${NEW_SDK_SHA256}/g" WORKSPACE
popd

# Host SHA update
NEW_HOST_SHA=$(bazel query //external:proxy_wasm_cpp_host --output=build | grep -Pom1 "https://github.com/proxy-wasm/proxy-wasm-cpp-host/archive/\K[a-zA-Z0-9]{40}")
NEW_HOST_SHA256=$(bazel query //external:proxy_wasm_cpp_host --output=build | grep -Pom1 "sha256 = \"\K[a-zA-Z0-9]{64}")
pushd $WD
CUR_HOST_SHA=$(bazel query //external:proxy_wasm_cpp_host --output=build | grep -Pom1 "https://github.com/proxy-wasm/proxy-wasm-cpp-host/archive/\K[a-zA-Z0-9]{40}")
CUR_HOST_SHA256=$(bazel query //external:proxy_wasm_cpp_host --output=build | grep -Pom1 "sha256 = \"\K[a-zA-Z0-9]{64}")
sed -i "s/${CUR_HOST_SHA}/${NEW_HOST_SHA}/g" bazel/wasm.bzl
sed -i "s/${CUR_HOST_SHA256}/${NEW_HOST_SHA256}/g" bazel/wasm.bzl
popd

# Istio proxy SHA update
ISTIO_PROXY_TMP_DIR=$(mktemp -d -t proxy-XXXXXXXXXX)
trap "rm -rf ${ISTIO_PROXY_TMP_DIR}" EXIT

cd ${ISTIO_PROXY_TMP_DIR}

if [[ ${RELEASE} == "master" ]]; then
  git clone --depth=1 --branch main https://github.com/istio/proxy
else
  git clone --depth=1 --branch release-${RELEASE} https://github.com/istio/proxy
fi
cd proxy

NEW_PROXY_SHA=$(git rev-parse HEAD)
NEW_PROXY_SHA256=$(wget https://github.com/istio/proxy/archive/${NEW_PROXY_SHA}.tar.gz && sha256sum ${NEW_PROXY_SHA}.tar.gz | grep -Pom1 "\K[a-zA-Z0-9]{64}")
trap "rm -rf ${NEW_PROXY_SHA}.tar.gz" EXIT
pushd $WD
CUR_PROXY_SHA=$(bazel query //external:io_istio_proxy --output=build | grep -Pom1 "https://github.com/istio/proxy/archive/\K[a-zA-Z0-9]{40}")
CUR_PROXY_SHA256=$(bazel query //external:io_istio_proxy --output=build | grep -Pom1 "sha256 = \"\K[a-zA-Z0-9]{64}")
sed -i "s/${CUR_PROXY_SHA}/${NEW_PROXY_SHA}/g" WORKSPACE
sed -i "s/${CUR_PROXY_SHA256}/${NEW_PROXY_SHA256}/g" WORKSPACE
popd
