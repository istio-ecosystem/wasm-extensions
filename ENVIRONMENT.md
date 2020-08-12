## Envoy dev environment

Set up [Bazel](bazel.build), an open-source build and test tool similar to Make, Maven, and Gradle.
```
sudo wget -O /usr/local/bin/bazel https://github.com/bazelbuild/bazelisk/releases/latest/download/bazelisk-linux-amd64
sudo chmod +x /usr/local/bin/bazel
sudo apt-get -y install libtool cmake automake autoconf make ninja-build curl unzip golang-go virtualenv libc++-dev lcov clangd-9
```
```
go get -u github.com/bazelbuild/buildtools/buildifier
go get -u github.com/bazelbuild/buildtools/buildozer
```
```
echo export BUILDIFIER_BIN=~/go/bin/buildifier >> ~/.bashrc
echo export BUILDOZER_BIN=~/go/bin/buildozer >> ~/.bashrc
echo export CLANG_FORMAT=/opt/llvm/bin/clang-format >> ~/.bashrc
chmod a+rx ~/go/bin/buildozer
chmod a+rx ~/go/bin/buildifier
```
Install `llvm` and `clang`:
```
wget https://github.com/llvm/llvm-project/releases/download/llvmorg-10.0.0/clang+llvm-10.0.0-x86_64-linux-gnu-ubuntu-18.04.tar.xz
tar xJvf clang+llvm-10.0.0-x86_64-linux-gnu-ubuntu-18.04.tar.xz
sudo mv clang+llvm-10.0.0-x86_64-linux-gnu-ubuntu-18.04 /opt/llvm
```

Set up `llvm` and `clang` for Envoy:
```
bazel/setup_clang.sh /opt/llvm
```
