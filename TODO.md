# TODO

## Features
- [ ] P0: MIS
- [ ] P0: Recursive refraction
- [ ] P0: Depth of field
- [ ] P0: Volumetrics
- [ ] P0: Features from "Ray tracing in a weekend" series
  - [ ] Ray tracing in a weekend
  - [ ] Ray tracing: the next week
  - [ ] Ray tracing: the rest of your life
- [ ] P1: Add denoiser (http://theinstructionlimit.com/unity-toy-path-tracer)
- [ ] P1: GUI scene viewer (imgui?)
- [ ] P1: Support for a common file format (obj, glTF, USD?)
- [ ] P3: Other camera types, like orthographic
- [ ] P3: More complex reflectance models, Torrance-Sparrow?
- [ ] P3: Measured BRDF support

## Scenes
- [ ] Utah teapot
- [ ] Stanford bunny
- [ ] pbrt scenes?

## Performance improvements
- [ ] P1: Add multithreading support
- [ ] SIMD support?
- [ ] P3: Include an end bound on ray intersections to avoid costly intersection
      tests that are further away than already-known intersections

## DONE
- [x] Add surface area heuristic to BVH
- [x] Acceleration structure (BVH)
- [x] Traverse BVH children in an intelligent way, updating max_distance to
      avoid intersecting farther bounding boxes. Use the sign of the ray's
      direction vector for the coordinate axis along which the BVH node was
      split
- [x] Per-vertex normals
- [x] Area light source / stratified sampling
- [x] Antialiasing
- [x] Cornell box
- [x] Stanford dragon
- [x] Tone mapping of the output image (gamma correction?)
- [x] Golden testing framework
- [x] P0: Allow separate rendering of direct and bounce light
