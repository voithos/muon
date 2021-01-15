# TODO

## Features
- [ ] Recursive refraction
- [ ] More complex reflectance models, Torrance-Sparrow?
- [ ] Other camera types, like orthographic
- [ ] Support for a common file format (obj, glTF, USD?)
- [ ] Tone mapping of the output image (gamma correction?)
- [ ] Allow separate rendering of direct and bounce light
- [ ] Depth of field
- [ ] Golden testing framework

## Scenes
- [ ] Utah teapot
- [ ] Stanford bunny
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
- [x] Per-vertex normals
- [x] Area light source / stratified sampling
- [x] Antialiasing
- [x] Cornell box
- [x] Stanford dragon
