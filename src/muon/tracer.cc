#include "muon/tracer.h"

#include <iostream>

#include "glog/logging.h"
#include "muon/eval.h"
#include "muon/film.h"
#include "muon/parser.h"
#include "muon/sampler.h"
#include "muon/scene.h"

namespace muon {

void trace(std::string scene_file) {
  Parser p(scene_file);
  Scene s = p.Parse();
  Evaluator e(s);
  Film f(s.width, s.height, s.output);

  Sampler sampler(s.width, s.height);

  float x, y;
  while (sampler.NextSample(x, y)) {
    VLOG_EVERY_N(2, 1000) << "Generated " << google::COUNTER
                          << "th sample: x=" << x << ", y=" << y;
    Ray r = s.camera->CastRay(x, y);
    glm::vec3 c = e.Eval(r);

    int px_x = x;
    int px_y = y;
    f.SetPixel(px_x, px_y, c);
  }

  f.WriteOutput();
}

} // namespace muon
