cc_library(
    name = "config",
    srcs = ["config.cc", "common.h"],
    hdrs = ["config.h"],
    deps = ["@github_nlohmann_json//:json"],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "http_parser",
    srcs = ["http_parser.cc", "common.h"],
    hdrs = ["http_parser.h"],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "sqli",
    srcs = ["sqli.cc", "common.h"],
    hdrs = ["sqli.h"],
    deps = ["@libinjection//:libinjection"],
    visibility = ["//visibility:public"],
)


