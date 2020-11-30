cc_binary(
    name = "muon",
    srcs = ["muon.cc"],
    deps = [
        ":tracer",
        "@com_google_absl//absl/flags:flag",
        "@com_google_absl//absl/flags:parse",
        "@com_google_absl//absl/flags:usage",
    ],
)

cc_library(
    name = "tracer",
    srcs = ["tracer.cc"],
    hdrs = ["tracer.h"],
    deps = [
        ":parser",
        ":scene",
        "//third_party/cimg",
    ],
)

cc_library(
    name = "film",
    srcs = ["film.cc"],
    hdrs = ["film.h"],
    deps = [
        "//third_party/cimg",
    ],
)

cc_library(
    name = "parser",
    srcs = ["parser.cc"],
    hdrs = ["parser.h"],
    deps = [
        ":scene",
        ":strings",
        "//third_party/glm",
        "@com_github_google_glog//:glog",
    ],
)

cc_library(
    name = "strings",
    hdrs = ["strings.h"],
)

cc_library(
    name = "scene",
    srcs = ["scene.cc"],
    hdrs = ["scene.h"],
    deps = [
        "//third_party/glm",
    ],
)
