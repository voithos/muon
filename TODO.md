# TODO

## Features
- [ ] Per-vertex normals
- [ ] Recursive refraction
- [ ] Area light source / stratified sampling
- [ ] Antialiasing
- [ ] More complex reflectance models, Torrance-Sparrow?
- [ ] Other camera types, like orthographic
- [ ] Support for a common file format (obj, glTF, USD?)

## Scenes
- [ ] Utah teapot
- [ ] Cornell box
- [ ] Stanford bunny
- [ ] Stanford dragon
- [ ] pbrt scenes?

## Performance improvements
- [ ] Include an end bound on ray intersections to avoid costly intersection
      tests that are further away than already-known intersections
- [ ] Add multithreading support

## DONE
- [x] Add surface area heuristic to BVH
- [x] Acceleration structure (BVH)
- [x] Traverse BVH children in an intelligent way, updating max_distance to
      avoid intersecting farther bounding boxes. Use the sign of the ray's
      direction vector for the coordinate axis along which the BVH node was
      split
