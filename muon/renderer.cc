#include "muon/renderer.h"

#include <iostream>

#include "glog/logging.h"
#include "muon/film.h"
#include "muon/parser.h"
#include "muon/sampler.h"
#include "muon/scene.h"
#include "muon/stats.h"
#include "muon/tracer.h"

namespace muon {

void Renderer::Render() {
  Stats stats;
  Parser parser(scene_file_, stats);
  Scene scene = parser.Parse();
  Film film(scene.width, scene.height, scene.output);

  Sampler sampler(scene.width, scene.height);
  stats.SetTotalSamples(sampler.TotalSamples());

  Tracer t(scene);

  // TODO: Refactor progress into separate class.
  int last_percent = 0;

  stats.Start();
  float x, y;
  while (sampler.NextSample(x, y)) {
    stats.IncrementSamples();

    float progress = stats.Progress();
    int percent = int(progress * 100);
    LOG_IF(INFO, percent > last_percent) << "Completion: " << percent << " %";
    last_percent = percent;

    Ray r = scene.camera->CastRay(x, y);
    glm::vec3 c = t.Trace(r);

    int px_x = x;
    int px_y = y;
    film.SetPixel(px_x, px_y, c);
  }
  stats.Stop();

  film.WriteOutput();

  if (show_stats_) {
    std::cerr << stats;
  }
}

} // namespace muon
