def scene_diff_test(name, scene, golden, truth=None, tolerance=None, size="medium"):
  """Creates a diff test for the given scene files."""
  extra_args = []
  extra_data = []
  if truth != None:
    # Nondeterministic test requested.
    if tolerance == None:
      fail("tolerance must be provided if a truth image is given")
    extra_args.append("$(location {})".format(truth))
    extra_args.append(tolerance)
    extra_data.append(truth)

  native.sh_test(
      name = name,
      size = size,
      srcs = ["//test:diff_test.sh"],
      args = [
          "$(location {})".format(scene),
          "$(location {})".format(golden),
      ] + extra_args,
      data = [
          golden,
          scene,
          "//muon",
      ] + extra_data,
      deps = [
          "@bazel_tools//tools/bash/runfiles",
      ],
  )
