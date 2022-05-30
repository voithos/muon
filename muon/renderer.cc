#include "muon/renderer.h"

#include <iostream>
#include <memory>
#include <thread>
#include <vector>

#include "glog/logging.h"
#include "muon/film.h"
#include "muon/integration.h"
#include "muon/parser.h"
#include "muon/sampling.h"
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
  Film film(sc.scene->width, sc.scene->height, sc.scene->pixel_samples,
            sc.scene->gamma, output);

  // Create more tiles than threads, so that the threads can better share the
  // workload in case certain parts of the image are more computationally
  // intense.
  int num_tiles = NumTiles(sc.scene->width, sc.scene->height,
                           sc.scene->pixel_samples, options_.parallelism);
  TileQueue tiles(TileImage(sc.scene->width, sc.scene->height, num_tiles,
                            *sc.scene->seedgen));

  // Launch render threads.
  std::vector<std::thread> threads;
  for (uint32_t thread_i = 0; thread_i < options_.parallelism; ++thread_i) {
    std::thread t([&sc, &tiles, &film, &stats] {
      // Clone the uninitialized integrator for this thread, and initialize it.
      std::unique_ptr<Integrator> integrator = sc.integrator_prototype->Clone();
      integrator->Init();

      absl::optional<Tile> tile;
      while ((tile = tiles.TryDequeue())) {
        Sampler sampler(tile.value(), sc.scene->pixel_samples);

        float x, y;
        while (sampler.NextSample(x, y)) {
          // TODO: Feed progress into a progress system.
          // float progress = sampler.Progress();

          Ray r = sc.scene->camera->CastRay(x, y);
          glm::vec3 c = integrator->Trace(r);

          int px_x = x;
          int px_y = y;
          film.SetPixel(px_x, px_y, c);
        }

        VLOG(2) << "Tile #" << tile->idx
                << " complete; remaining tiles: " << tiles.size();
      }

      stats.AddTraceStats(integrator->trace_stats());
    });
    threads.push_back(std::move(t));
  }

  // Rejoin render threads.
  for (std::thread& t : threads) {
    t.join();
  }
  stats.Stop();

  VLOG(2) << "Render threads done; writing output";
  film.WriteOutput();

  if (options_.show_stats) {
    std::cerr << stats;
  }
}

}  // namespace muon
