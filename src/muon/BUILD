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
