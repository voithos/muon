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
  Film film(sc.scene->width, sc.scene->height, sc.scene->samples_per_pixel,
            sc.scene->gamma, output);

  // TODO: Create more tiles than threads, so that the threads can better share
  // the workload in case certain parts of the image are more computationally
  // intense.
  TileQueue tiles(TileImage(sc.scene->width, sc.scene->height,
                            /*num_tiles=*/options_.parallelism));

  // Launch render threads.
  std::vector<std::thread> threads;
  for (int thread_i = 0; thread_i < options_.parallelism; ++thread_i) {
    std::thread t([&sc, &tiles, &film, &stats] {
      // Clone the uninitialized integrator for this thread, and initialize it.
      std::unique_ptr<Integrator> integrator = sc.integrator_prototype->Clone();
      integrator->Init();

      absl::optional<Tile> tile;
      while ((tile = tiles.TryDequeue())) {
        Sampler sampler(tile.value(), sc.scene->samples_per_pixel);

        // TODO: Refactor progress into separate class.
        int last_percent = 0;

        float x, y;
        while (sampler.NextSample(x, y)) {
          float progress = sampler.Progress();
          int percent = int(progress * 100);
          LOG_IF(INFO, percent > last_percent)
              << "Tile #" << tile->idx << " completion: " << percent << " %";
          last_percent = percent;

          Ray r = sc.scene->camera->CastRay(x, y);
          glm::vec3 c = integrator->Trace(r);

          int px_x = x;
          int px_y = y;
          film.SetPixel(px_x, px_y, c);
        }
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

  film.WriteOutput();

  if (options_.show_stats) {
    std::cerr << stats;
  }
}

}  // namespace muon
