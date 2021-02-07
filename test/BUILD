sh_test(
    name = "tri_test",
    size = "medium",
    srcs = ["diff_test.sh"],
    args = ["tri.test"],
    data = [
        "testdata/tri.png",
        "tri.test",
        "//muon",
    ],
    deps = [
        "@bazel_tools//tools/bash/runfiles",
    ],
)
