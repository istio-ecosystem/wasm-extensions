# Set up Develop Environment

## C++ Extension Set Up

### Install Bazelisk as Bazel

It is recommended to use Bazelisk installed as bazel:

On Linux, run the following commands:

```
sudo wget -O /usr/local/bin/bazel https://github.com/bazelbuild/bazelisk/releases/latest/download/bazelisk-linux-amd64
sudo chmod +x /usr/local/bin/bazel
```

On MacOS, you can `brew install bazelisk`. This adds both `bazelisk` and `bazel` to the `PATH`.

For additional installation methods such as using `npm` and advanced configuration see the [official Bazelisk Installation Guide](https://github.com/bazelbuild/bazelisk#installation).

### Installing Minimum Dependencies

Several dependencies are needs in order to build a C++ WebAssembly extensions with Bazel.

on Unbuntu, run the following command:

```
sudo apt-get install gcc curl python3
```
