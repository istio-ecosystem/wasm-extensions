#!/bin/bash

set -eu

# Update example extension and various guide with SDK SHA the given release.
# The files touched by this script are:
# * Example extension config example file `example/config/example-filter.yaml`, which has the module download link and checksum.
# * Example extension config integration test `example/test/example_test.go`, which has the proxy version to download for integration testing.
# * Wasm extension C++ Walkthrough `doc/write-a-wasm-extension-with-cpp.md`, which has the proxy wasm cpp sdk SHA and checksum.
# * Wasm extension integration test guide `doc/write-integration-test.md`, which has the proxy version to download for integration testing.
# * Basic auth config example file `basic_auth/config/gateway-filter.yaml`, which has the module download link and checksum.
function usage() {
  echo "$0
    -r the release branch that this script should look at, e.g. 1.8, master."
  exit 1
}

function semverParseInto() {
    local RE='[^0-9]*\([0-9]*\)[.]\([0-9]*\)[.]\([0-9]*\)\([0-9A-Za-z-]*\)'
    #MAJOR
    eval $2=`echo $1 | sed -e "s#$RE#\1#"`
    #MINOR
    eval $3=`echo $1 | sed -e "s#$RE#\2#"`
}

RELEASE=""

while getopts r: arg ; do
  case "${arg}" in
    r) RELEASE="${OPTARG}";;
    *) usage;;
  esac
done

if [[ ${RELEASE} == "" ]]; then
    echo "release branch has to be provided. e.g. update-guide.sh -r 1.8"
    exit 1
fi

# Clone istio-envoy repo, get and update proxy Wasm sdk and host SHA
ENVOY_TMP_DIR=$(mktemp -d -t envoy-XXXXXXXXXX)
trap "rm -rf ${ENVOY_TMP_DIR}" EXIT

pushd ${ENVOY_TMP_DIR}

git clone --depth=1 https://github.com/envoyproxy/envoy
cd envoy
git remote add istio-envoy https://github.com/istio/envoy
git fetch istio-envoy
cd envoy

NEW_SDK_SHA=$(bazel query //external:proxy_wasm_cpp_sdk --output=build | grep -Pom1 "https://github.com/proxy-wasm/proxy-wasm-cpp-sdk/archive/\K[a-zA-Z0-9]{40}")
NEW_SDK_SHA256=$(bazel query //external:proxy_wasm_cpp_sdk --output=build | grep -Pom1 "sha256 = \"\K[a-zA-Z0-9]{64}")

popd

# Update example WORKSPACE, integration test proxy version
sed -e "s|PROXY_WASM_CPP_SDK_SHA = .*|PROXY_WASM_CPP_SDK_SHA = \"${NEW_SDK_SHA}\"|" -i example/WORKSPACE
sed -e "s|PROXY_WASM_CPP_SDK_SHA256 = .*|PROXY_WASM_CPP_SDK_SHA256 = \"${NEW_SDK_SHA256}\"|" -i example/WORKSPACE
sed -e "s|DownloadVersion:.*|DownloadVersion: \"${RELEASE}\",|" -i example/test/example_test.go

semverParseInto ${RELEASE}.0 MAJOR MINOR PATCH
# Update basic auth config with wasm file of the new version.
BASIC_AUTH_WASM_MODULE_URL="https://github.com/istio-ecosystem/wasm-extensions/releases/download/${RELEASE}.0/basic-auth.wasm"
BASIC_AUTH_MODULE_CHECKSUM=$(wget ${BASIC_AUTH_WASM_MODULE_URL} && sha256sum basic-auth.wasm | cut -d' ' -f 1)
trap "rm -rf basic-auth.wasm" EXIT
sed -e "s|uri: .*|uri: ${BASIC_AUTH_WASM_MODULE_URL}|" -i extensions/basic_auth/config/gateway-filter.yaml
sed -e "s|sha256: .*|sha256: ${BASIC_AUTH_MODULE_CHECKSUM}|" -i extensions/basic_auth/config/gateway-filter.yaml
sed -e "s|proxyVersion: .*|proxyVersion: ^${MAJOR}\\\.${MINOR}.*|" -i extensions/basic_auth/config/gateway-filter.yaml
rm -rf basic-auth.wasm

# Update guide doc
sed -e "s|PROXY_WASM_CPP_SDK_SHA = .*|PROXY_WASM_CPP_SDK_SHA = \"${NEW_SDK_SHA}\"|" -i doc/write-a-wasm-extension-with-cpp.md
sed -e "s|PROXY_WASM_CPP_SDK_SHA256 = .*|PROXY_WASM_CPP_SDK_SHA256 = \"${NEW_SDK_SHA256}\"|" -i doc/write-a-wasm-extension-with-cpp.md
sed -e "s|DownloadVersion:.*|DownloadVersion: \"${RELEASE}\",|" -i doc/write-integration-test.md
