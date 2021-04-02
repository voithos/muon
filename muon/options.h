#ifndef MUON_OPTIONS_H_
#define MUON_OPTIONS_H_

#include <string>

#include "muon/acceleration_type.h"

namespace muon {

// Input options for the renderer.
struct Options {
  // The path to the output file. Overrides the output specified in the scene
  // file.
  std::string output;
  // The acceleration structure to use.
  AccelerationType acceleration;
  // The strategy to use when partitioning primitives in a BVH.
  PartitionStrategy partition_strategy;
  // The number of parallel threads to use when rendering.
  uint32_t parallelism;
  // Whether or not to show stats.
  bool show_stats;
};

}  // namespace muon

#endif
