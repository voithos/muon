load("//test:build_defs.bzl", "scene_diff_test")

scene_diff_test(
    name = "intersection_test",
    golden = "testdata/intersection.png",
    scene = "intersection.test",
)

scene_diff_test(
    name = "transforms_test",
    golden = "testdata/transforms.png",
    scene = "transforms.test",
)

scene_diff_test(
    name = "spheres_raytrace_test",
    golden = "testdata/spheres_raytrace.png",
    scene = "spheres_raytrace.test",
)

scene_diff_test(
    name = "cornell_raytrace_test",
    golden = "testdata/cornell_raytrace.png",
    scene = "cornell_raytrace.test",
)

scene_diff_test(
    name = "analytic_test",
    golden = "testdata/analytic.png",
    scene = "analytic.test",
)

scene_diff_test(
    name = "cornell_normals_test",
    golden = "testdata/cornell_normals.png",
    scene = "cornell_normals.test",
)

scene_diff_test(
    name = "cornell_depth_test",
    golden = "testdata/cornell_depth.png",
    scene = "cornell_depth.test",
)

scene_diff_test(
    name = "sphere_test",
    golden = "testdata/sphere_golden.png",
    scene = "sphere.test",
    tolerance = "0.0005",
    truth = "testdata/sphere_truth.png",
)
