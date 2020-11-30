package(default_visibility = ["//visibility:public"])

cc_library(
    name = "cimg",
    hdrs = ["CImg.h"],
    copts = [
        "-Dcimg_display=1",
        "-Dcimg_use_vt100",
    ],
    linkopts = [
        "-lm",
        "-lX11",
        "-lpthread",
    ],
)
