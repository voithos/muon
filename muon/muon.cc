#include <iostream>
#include <string>

#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "absl/flags/usage.h"
#include "glog/logging.h"
#include "muon/renderer.h"

ABSL_FLAG(std::string, scene, "", "Path to a scene file");
ABSL_FLAG(bool, stats, true, "Whether to show stats after rendering");

int main(int argc, char **argv) {
  // Initialize Google logging framework. absl doesn't yet have a logging
  // system, so the flags that this would normally register don't work.
  // Instead, one can use env vars for debugging:
  //   Log to stderr:        GLOG_logtostderr=1
  //   Include verbose logs: GLOG_v=2
  google::InitGoogleLogging(argv[0]);

  absl::SetProgramUsageMessage(
      "Subatomic ray tracer. Usage:\n  muon --scene path/to/scene.file");
  absl::ParseCommandLine(argc, argv);

  std::string scene_file = absl::GetFlag(FLAGS_scene);
  if (scene_file == "") {
    LOG(ERROR) << "A scene file is required";
    return 1;
  }
  bool stats = absl::GetFlag(FLAGS_stats);

  muon::Renderer r(scene_file, stats);
  r.Render();

  return 0;
}
