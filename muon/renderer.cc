#include "muon/renderer.h"

#include <iostream>

#include "glog/logging.h"
#include "muon/film.h"
#include "muon/integration.h"
#include "muon/parser.h"
#include "muon/sampler.h"
#include "muon/scene.h"
#include "muon/stats.h"

namespace muon {

void Renderer::Render() const {
  Stats stats;
  stats.Start();

  Parser parser(scene_file_, options_);
  SceneConfig sc = parser.Parse();
  stats.BuildComplete();

  const std::string& output =
      options_.output != "" ? options_.output : sc.scene->output;
  Film film(sc.scene->width, sc.scene->height, sc.scene->samples_per_pixel,
            sc.scene->gamma, output);

  TileQueue tiles(
      TileImage(sc.scene->width, sc.scene->height, /*num_tiles=*/1));
  Sampler sampler(tiles.TryDequeue().value(), sc.scene->samples_per_pixel);

  // TODO: Refactor progress into separate class.
  int last_percent = 0;

  float x, y;
  while (sampler.NextSample(x, y)) {
    float progress = sampler.Progress();
    int percent = int(progress * 100);
    LOG_IF(INFO, percent > last_percent) << "Completion: " << percent << " %";
    last_percent = percent;

    Ray r = sc.scene->camera->CastRay(x, y);
    glm::vec3 c = sc.integrator->Trace(r);

    int px_x = x;
    int px_y = y;
    film.SetPixel(px_x, px_y, c);
  }
  stats.Stop();
  stats.AddTraceStats(sc.integrator->trace_stats());

  film.WriteOutput();

  if (options_.show_stats) {
    std::cerr << stats;
  }
}

}  // namespace muon
