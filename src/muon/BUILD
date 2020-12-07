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
        ":eval",
        ":film",
        ":parser",
        ":sampler",
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
        "//third_party/glm",
        "@com_github_google_glog//:glog",
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
        "@com_google_absl//absl/memory:memory",
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
        ":camera",
        ":lighting",
        ":strings",
        "//third_party/glm",
        "@com_github_google_glog//:glog",
        "@com_google_absl//absl/types:optional",
    ],
)

cc_library(
    name = "camera",
    srcs = ["camera.cc"],
    hdrs = ["camera.h"],
    deps = [
        "//third_party/glm",
    ],
)

cc_library(
    name = "sampler",
    srcs = ["sampler.cc"],
    hdrs = ["sampler.h"],
    deps = [
    ],
)

cc_library(
    name = "eval",
    srcs = ["eval.cc"],
    hdrs = ["eval.h"],
    deps = [
        ":camera",
        ":scene",
        "//third_party/glm",
    ],
)

cc_library(
    name = "lighting",
    srcs = ["lighting.cc"],
    hdrs = ["lighting.h"],
    deps = [
        "//third_party/glm",
    ],
)
