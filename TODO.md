# TODO

## Features
- [ ] P0: Recursive refraction
- [ ] P0: Depth of field
- [ ] P0: Features from "Ray tracing in a weekend" series
  - [ ] Ray tracing in a weekend
  - [ ] Ray tracing: the next week
  - [ ] Ray tracing: the rest of your life
- [ ] P1: Volumetrics
- [ ] P1: Textures
- [ ] P2: Add denoiser (https://github.com/OpenImageDenoise/oidn)
- [ ] P3: Other camera types, like orthographic
- [ ] P3: Disney BRDF
- [ ] P3: GUI scene viewer (imgui?)
- [ ] P3: Allow configuration of random seed

## Scenes
- [ ] Utah teapot
- [ ] Stanford bunny
- [ ] pbrt scenes? https://benedikt-bitterli.me/resources/

## Performance improvements
- [ ] SIMD support?
- [ ] P3: Adaptive sampling
- [ ] P3: Bidirectional path tracing
- [ ] P3: Include an end bound on ray intersections to avoid costly intersection
      tests that are further away than already-known intersections
- [ ] P4: Metropolis light transport

## Bugs
- [ ] P1: Some parts of the lighting calculations don't take orientation into
      account (so lights illuminate things behind them)
- [ ] P1: Roughness cannot be set to 0 (or 1?) without black results

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
- [x] P1: Add multithreading support
- [x] P0: MIS
- [x] P1: Support for a common file format (obj, glTF, USD?)
