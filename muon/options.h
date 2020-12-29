#ifndef MUON_OPTIONS_H_
#define MUON_OPTIONS_H_

#include <string>

#include "muon/acceleration_type.h"

namespace muon {

// Input options for the renderer.
struct Options {
  // The acceleration structure to use.
  AccelerationType acceleration;
  // The strategy to use when partitioning primitives in a BVH.
  PartitionStrategy partition_strategy;
  // Whether or not to show stats.
  bool show_stats;
};

}  // namespace muon

#endif
