def scene_diff_test(name, scene, golden, size="medium"):
  """Creates a diff test for the given scene files."""
  native.sh_test(
      name = name,
      size = size,
      srcs = ["//test:diff_test.sh"],
      args = [
          "$(location {})".format(scene),
          "$(location {})".format(golden),
      ],
      data = [
          golden,
          scene,
          "//muon",
      ],
      deps = [
          "@bazel_tools//tools/bash/runfiles",
      ],
  )
