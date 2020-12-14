cc_binary(
    name = "muon",
    srcs = ["muon.cc"],
    deps = [
        ":renderer",
        "@com_google_absl//absl/flags:flag",
        "@com_google_absl//absl/flags:parse",
        "@com_google_absl//absl/flags:usage",
    ],
)

cc_library(
    name = "renderer",
    srcs = ["renderer.cc"],
    hdrs = ["renderer.h"],
    deps = [
        ":film",
        ":parser",
        ":sampler",
        ":scene",
        ":stats",
        ":tracer",
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
        ":lighting",
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
        ":types",
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
    name = "tracer",
    srcs = ["tracer.cc"],
    hdrs = ["tracer.h"],
    deps = [
        ":camera",
        ":lighting",
        ":scene",
        ":stats",
        "//third_party/glm",
    ],
)

cc_library(
    name = "lighting",
    srcs = ["lighting.cc"],
    hdrs = ["lighting.h"],
    deps = [
        ":camera",
        ":objects",
        ":types",
        "//third_party/glm",
    ],
)

cc_library(
    name = "types",
    hdrs = ["types.h"],
)

cc_library(
    name = "objects",
    srcs = ["objects.cc"],
    hdrs = ["objects.h"],
    deps = [
        ":camera",
        ":types",
        "@com_google_absl//absl/types:optional",
    ],
)

cc_library(
    name = "stats",
    srcs = ["stats.cc"],
    hdrs = ["stats.h"],
    deps = [
    ],
)