#include "muon/stats.h"

#include <iomanip>
#include <ostream>

namespace muon {

void Stats::SetTotalSamples(uint64_t total_samples) {
  total_samples_ = total_samples;
}

void Stats::Start() { start_time_ = std::chrono::steady_clock::now(); }

void Stats::BuildComplete() {
  build_complete_time_ = std::chrono::steady_clock::now();
}

void Stats::Stop() { end_time_ = std::chrono::steady_clock::now(); }

float Stats::Progress() const {
  return samples_so_far_ / static_cast<float>(total_samples_);
}

constexpr int kLabelWidth = 18;
constexpr int kFieldWidth = 12;
constexpr int kLineWidth = kLabelWidth + kFieldWidth + 12;

std::ostream& Label(std::ostream& os) {
  return os << std::setw(kLabelWidth) << std::left;
}

std::ostream& Field(std::ostream& os) {
  return os << std::setw(kFieldWidth) << std::right;
}

std::ostream& operator<<(std::ostream& os, const Stats& stats) {
  std::chrono::duration<float> duration = stats.end_time_ - stats.start_time_;
  std::chrono::duration<float> build_duration =
      stats.build_complete_time_ - stats.start_time_;
  std::chrono::duration<float> render_duration =
      stats.end_time_ - stats.build_complete_time_;
  double object_hit_rate =
      (stats.object_hits_ / static_cast<double>(stats.object_tests_)) * 100.0f;
  double bounds_hit_rate =
      (stats.bounds_hits_ / static_cast<double>(stats.bounds_tests_)) * 100.0f;

  os << std::setw(kLineWidth) << std::setfill('-') << ">>" << std::setfill(' ')
     << std::endl;
  os << Label << "Total time"
     << " : " << Field << std::fixed << std::setprecision(2) << duration.count()
     << " (sec)" << std::endl;
  os << Label << "Build time"
     << " : " << Field << std::fixed << std::setprecision(2)
     << build_duration.count() << " (sec)" << std::endl;
  os << Label << "Render time"
     << " : " << Field << std::fixed << std::setprecision(2)
     << render_duration.count() << " (sec)" << std::endl;
  os << Label << "Samples"
     << " : " << Field << stats.samples_so_far_ << std::endl;
  os << Label << "Ray-object tests"
     << " : " << Field << stats.object_tests_ << std::endl;
  os << Label << "Ray-object hits"
     << " : " << Field << stats.object_hits_ << std::endl;
  os << Label << "Hit rate"
     << " : " << Field << std::fixed << std::setprecision(2) << object_hit_rate
     << " %" << std::endl;
  os << Label << "Bounds tests"
     << " : " << Field << stats.bounds_tests_ << std::endl;
  os << Label << "Bounds hits"
     << " : " << Field << stats.bounds_hits_ << std::endl;
  os << Label << "Bounds hit rate"
     << " : " << Field << std::fixed << std::setprecision(2) << bounds_hit_rate
     << " %" << std::endl;
  os << std::setw(kLineWidth) << std::setfill('-') << ">>" << std::setfill(' ')
     << std::endl;
  return os;
}

}  // namespace muon
