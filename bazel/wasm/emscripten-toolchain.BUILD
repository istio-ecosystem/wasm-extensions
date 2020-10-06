licenses(["notice"])  # Apache 2

package(default_visibility = ["//visibility:public"])

filegroup(
    name = "all",
    srcs = glob(
        ["**"],
        exclude = [
            "upstream/emscripten/cache/is_vanilla.txt",
            ".emscripten_sanity",
        ]),
)
