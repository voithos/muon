#include <iostream>
#include <string>

#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "absl/flags/usage.h"
#include "muon/tracer.h"

ABSL_FLAG(std::string, scene, "", "Path to a scene file");

int main(int argc, char **argv) {
  absl::SetProgramUsageMessage(
      "Subatomic ray tracer. Usage:\n  muon --scene path/to/scene.file");
  absl::ParseCommandLine(argc, argv);

  muon::trace(absl::GetFlag(FLAGS_scene));

  return 0;
}
