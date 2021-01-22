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
        ":integration",
        ":options",
        ":parser",
        ":sampler",
        ":scene",
        ":stats",
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
        "@com_google_absl//absl/strings:str_format",
    ],
)

cc_library(
    name = "parser",
    srcs = ["parser.cc"],
    hdrs = ["parser.h"],
    deps = [
        ":acceleration",
        ":acceleration_type",
        ":brdf_type",
        ":defaults",
        ":integration",
        ":lighting",
        ":materials",
        ":options",
        ":scene",
        ":stats",
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
        ":importance_sampling",
        ":lighting",
        ":objects",
        ":strings",
        ":types",
        ":vertex",
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
        ":ray",
        ":transform",
        "//third_party/glm",
    ],
)

cc_library(
    name = "sampler",
    srcs = ["sampler.cc"],
    hdrs = ["sampler.h"],
    deps = [
        ":random",
        "@com_github_google_glog//:glog",
    ],
)

cc_library(
    name = "integration",
    srcs = ["integration.cc"],
    hdrs = ["integration.h"],
    deps = [
        ":camera",
        ":hemisphere_sampling",
        ":lighting",
        ":random",
        ":scene",
        ":stats",
        ":transform",
        "//third_party/glm",
    ],
)

cc_library(
    name = "lighting",
    srcs = ["lighting.cc"],
    hdrs = ["lighting.h"],
    deps = [
        ":camera",
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
        ":bounds",
        ":camera",
        ":materials",
        ":strings",
        ":transform",
        ":types",
        ":vertex",
        "@com_github_google_glog//:glog",
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

cc_library(
    name = "defaults",
    hdrs = ["defaults.h"],
    deps = [
        ":brdf_type",
        ":importance_sampling",
        "//third_party/glm",
    ],
)

cc_library(
    name = "acceleration",
    srcs = ["acceleration.cc"],
    hdrs = ["acceleration.h"],
    deps = [
        ":acceleration_type",
        ":bounds",
        ":objects",
        ":stats",
        "@com_google_absl//absl/types:optional",
    ],
)

cc_library(
    name = "acceleration_type",
    srcs = ["acceleration_type.cc"],
    hdrs = ["acceleration_type.h"],
    deps = [
        "@com_google_absl//absl/flags:flag",
    ],
)

cc_library(
    name = "bounds",
    srcs = ["bounds.cc"],
    hdrs = ["bounds.h"],
    deps = [
        ":ray",
        ":transform",
        "//third_party/glm",
    ],
)

cc_library(
    name = "transform",
    srcs = ["transform.cc"],
    hdrs = ["transform.h"],
    deps = [
        "//third_party/glm",
    ],
)

cc_library(
    name = "ray",
    srcs = ["ray.cc"],
    hdrs = ["ray.h"],
    deps = [
        ":transform",
        "//third_party/glm",
    ],
)

cc_library(
    name = "options",
    hdrs = ["options.h"],
    deps = [
        ":acceleration_type",
    ],
)

cc_library(
    name = "vertex",
    hdrs = ["vertex.h"],
    deps = [
        "//third_party/glm",
    ],
)

cc_library(
    name = "materials",
    srcs = ["materials.cc"],
    hdrs = ["materials.h"],
    deps = [
        ":hemisphere_sampling",
        ":random",
        "//third_party/glm",
        "@com_google_absl//absl/types:optional",
    ],
)

cc_library(
    name = "random",
    srcs = ["random.cc"],
    hdrs = ["random.h"],
    deps = [
        "@com_github_google_glog//:glog",
    ],
)

cc_library(
    name = "importance_sampling",
    hdrs = ["importance_sampling.h"],
    deps = [
    ],
)

cc_library(
    name = "hemisphere_sampling",
    srcs = ["hemisphere_sampling.cc"],
    hdrs = ["hemisphere_sampling.h"],
    deps = [
        ":random",
        ":transform",
        "//third_party/glm",
    ],
)

cc_library(
    name = "brdf_type",
    hdrs = ["brdf_type.h"],
    deps = [
    ],
)
