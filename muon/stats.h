#ifndef MUON_STATS_H_
#define MUON_STATS_H_

#include <chrono>
#include <cstdint>
#include <string>

namespace muon {

// Records statistics about the tracer.
class Stats {
 public:
  void Start();
  void BuildComplete();
  void Stop();

  void IncrementPrimaryRays() { ++primary_rays_; }
  void IncrementSecondaryRays() { ++secondary_rays_; }
  void IncrementObjectTests() { ++object_tests_; }
  void IncrementObjectHits() { ++object_hits_; }
  void IncrementBoundsTests() { ++bounds_tests_; }
  void IncrementBoundsHits() { ++bounds_hits_; }

 private:
  uint64_t primary_rays_ = 0;
  uint64_t secondary_rays_ = 0;
  uint64_t object_tests_ = 0;
  uint64_t object_hits_ = 0;
  uint64_t bounds_tests_ = 0;
  uint64_t bounds_hits_ = 0;

  std::chrono::steady_clock::time_point start_time_;
  std::chrono::steady_clock::time_point build_complete_time_;
  std::chrono::steady_clock::time_point end_time_;

  friend std::ostream &operator<<(std::ostream &os, const Stats &stats);
};

}  // namespace muon

#endif
