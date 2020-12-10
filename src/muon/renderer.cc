#include "muon/renderer.h"

#include <iostream>

#include "glog/logging.h"
#include "muon/film.h"
#include "muon/parser.h"
#include "muon/sampler.h"
#include "muon/scene.h"
#include "muon/tracer.h"

namespace muon {

void render(std::string scene_file) {
  Parser p(scene_file);
  Scene s = p.Parse();
  Tracer t(s);
  Film f(s.width, s.height, s.output);

  Sampler sampler(s.width, s.height);
  // TODO: Refactor progress into separate class.
  int total_samples = sampler.TotalSamples();
  int last_percent = 0;

  float x, y;
  while (sampler.NextSample(x, y)) {
    VLOG_EVERY_N(2, 10000) << "Generated " << google::COUNTER
                           << "th sample: x=" << x << ", y=" << y;

    float progress =
        sampler.RequestedSamples() / static_cast<float>(total_samples);
    int percent = int(progress * 100);
    LOG_IF(INFO, percent > last_percent) << "Completion: " << percent << " %";
    last_percent = percent;

    Ray r = s.camera->CastRay(x, y);
    glm::vec3 c = t.Trace(r);

    int px_x = x;
    int px_y = y;
    f.SetPixel(px_x, px_y, c);
  }

  f.WriteOutput();
}

} // namespace muon
