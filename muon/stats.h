#ifndef MUON_STATS_H_
#define MUON_STATS_H_

#include <chrono>
#include <cstdint>
#include <string>

namespace muon {

// Records statistics about the tracer.
class Stats {
public:
  // Sets the total number of expected samples.
  // Must be called before any other method.
  void SetTotalSamples(uint64_t total_samples);

  void Start();
  void Stop();

  float Progress() const;

  void IncrementSamples() { ++samples_so_far_; }
  void IncrementObjectTests() { ++object_tests_; }
  void IncrementObjectHits() { ++object_hits_; }
  void IncrementBoundsTests() { ++bounds_tests_; }
  void IncrementBoundsHits() { ++bounds_hits_; }

private:
  uint64_t total_samples_ = 0;
  uint64_t samples_so_far_ = 0;
  uint64_t object_tests_ = 0;
  uint64_t object_hits_ = 0;
  uint64_t bounds_tests_ = 0;
  uint64_t bounds_hits_ = 0;

  std::chrono::steady_clock::time_point start_time_;
  std::chrono::steady_clock::time_point end_time_;

  friend std::ostream &operator<<(std::ostream &os, const Stats &stats);
};

} // namespace muon

#endif
