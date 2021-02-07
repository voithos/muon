def scene_diff_tests(scenes, size="medium"):
  """Creates diff tests for the given scene files.

  This assumes that each test file has a corresponding golden PNG file under a
  testdata/ directory.
  """
  for scene in scenes:
    prefix = scene.rsplit('.', 2)[0]
    name = prefix + '_test'
    golden = "testdata/{}.png".format(prefix)

    native.sh_test(
        name = name,
        size = size,
        srcs = ["//test:diff_test.sh"],
        args = [scene],
        data = [
            golden,
            scene,
            "//muon",
        ],
        deps = [
            "@bazel_tools//tools/bash/runfiles",
        ],
    )
