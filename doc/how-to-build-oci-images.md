# How to build Istio Wasm Plugin compatible OCI images

As of 1.12, Istio has supports for OCI images in its Wasm Plugin mechanism, and users are able to specify any OCI registry location in Wasm Plugin API resources where they store Wasm plugins as containers.

This document describes how to build OCI images which are consumable by Istio.

## Overview

There are two types of OCI images are supported by Istio. One is in the Docker format, and another is the standard OCI specification compliant format. Please note that both of them are supported by any OCI registries, and you can operate on standard CLI tools e.g. Docker CLI, [buildah](https://buildah.io/), etc. You can choose either format depending on your preference, and both types of containers are consumable by Istio Wasm Plugin API.

For the formal specification, please refer to [the link here](https://github.com/solo-io/wasm/blob/master/spec/spec-compat.md).

## Build Istio compatible Docker image 

We assume that you have a valid Wasm binary named `plugin.wasm`.

1. First, we prepare the following Dockerfile:

```
$ cat Dockerfile
FROM scratch

COPY plugin.wasm ./
```

**Note: you must have exactly one `COPY` instruction in the Dockerfile in order to end up having only one layer in produced images.**

2. Then, build your image via `docker build` command

```
$ docker build . -t my-registry/mywasm:0.1.0
```

3. Finally, push the image to your registry via `docker push` command

```
$ docker push my-registry/mywasm:0.1.0
```

## Build Istio compatible OCI image

We assume that you have a valid Wasm binary named `plugin.wasm` that you want to package as an image.

1. First, we create a working container from `scratch` base image with `buildah from` command.

```
$ buildah --name mywasm from scratch
mywasm
```

2. Then copy the Wasm binary into that base image by `buildah copy` command to create the layer.

```
$ buildah copy mywasm plugin.wasm ./
af82a227630327c24026d7c6d3057c3d5478b14426b74c547df011ca5f23d271
```

**Note: you must execute `buildah copy` exactly once in order to end up having only one layer in produced images**

4. Now, you can build a *compat* image and push it to your registry via `buildah commit` command

```
$ buildah commit mywasm docker://my-remote-registry/mywasm:0.1.0
```
