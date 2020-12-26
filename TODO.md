# TODO

## Features
- [ ] Per-vertex normals
- [ ] Recursive refraction
- [ ] Area light source / stratified sampling
- [ ] Antialiasing
- [ ] More complex reflectance models, Torrance-Sparrow?
- [ ] Other camera types, like orthographic
- [ ] Support for a common file format (USD?)

## Performance improvements
- [ ] Add surface area heuristic to BVH
- [ ] Include an end bound on ray intersections to avoid costly intersection
      tests that are further away than already-known intersections

## DONE
- [x] Acceleration structure (BVH)
