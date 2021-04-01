#ifndef MUON_STATS_H_
#define MUON_STATS_H_

#include <chrono>
#include <cstdint>
#include <mutex>
#include <string>

namespace muon {

class TraceStats {
 public:
  void IncrementPrimaryRays() { ++primary_rays_; }
  void IncrementSecondaryRays() { ++secondary_rays_; }
  void IncrementObjectTests() { ++object_tests_; }
  void IncrementObjectHits() { ++object_hits_; }
  void IncrementBoundsTests() { ++bounds_tests_; }
  void IncrementBoundsHits() { ++bounds_hits_; }

  uint64_t primary_rays() const { return primary_rays_; }
  uint64_t secondary_rays() const { return secondary_rays_; }
  uint64_t object_tests() const { return object_tests_; }
  uint64_t object_hits() const { return object_hits_; }
  uint64_t bounds_tests() const { return bounds_tests_; }
  uint64_t bounds_hits() const { return bounds_hits_; }

  TraceStats &operator+=(const TraceStats &rhs) {
    primary_rays_ += rhs.primary_rays_;
    secondary_rays_ += rhs.secondary_rays_;
    object_tests_ += rhs.object_tests_;
    object_hits_ += rhs.object_hits_;
    bounds_tests_ += rhs.bounds_tests_;
    bounds_hits_ += rhs.bounds_hits_;
    return *this;
  }

 private:
  uint64_t primary_rays_ = 0;
  uint64_t secondary_rays_ = 0;
  uint64_t object_tests_ = 0;
  uint64_t object_hits_ = 0;
  uint64_t bounds_tests_ = 0;
  uint64_t bounds_hits_ = 0;
};

// Records statistics about the tracer. Thread safe.
class Stats {
 public:
  void Start();
  void BuildComplete();
  void Stop();
  void AddTraceStats(const TraceStats &ts);

 private:
  TraceStats trace_;

  std::chrono::steady_clock::time_point start_time_;
  std::chrono::steady_clock::time_point build_complete_time_;
  std::chrono::steady_clock::time_point end_time_;

  mutable std::mutex mutex_;

  friend std::ostream &operator<<(std::ostream &os, const Stats &stats);
};

}  // namespace muon

#endif
