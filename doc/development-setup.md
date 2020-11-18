# Set up Develop Environment

## C++ Extension Set Up

### Install Bazelisk as Bazel

It is recommended to use Bazelisk installed as bazel:

On Linux, run the following commands:

```
sudo wget -O /usr/local/bin/bazel https://github.com/bazelbuild/bazelisk/releases/latest/download/bazelisk-linux-amd64
sudo chmod +x /usr/local/bin/bazel
```

### Installing Minimum Dependencies

Several dependencies are needs in order to build a C++ WebAssembly extensions with Bazel.

on Unbuntu, run the following command:

```
sudo apt-get install gcc curl python3
```
