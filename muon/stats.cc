#include "muon/stats.h"

#include <iomanip>
#include <ostream>

namespace muon {

void Stats::Start() { start_time_ = std::chrono::steady_clock::now(); }

void Stats::Stop() { end_time_ = std::chrono::steady_clock::now(); }

float Stats::Progress() const {
  return samples_so_far_ / static_cast<float>(total_samples_);
}

std::ostream &operator<<(std::ostream &os, const Stats &stats) {
  std::chrono::duration<float> duration = stats.end_time_ - stats.start_time_;

  os << "----------------------------------->>" << std::endl;
  os << "Render time      : " << std::fixed << std::setprecision(2)
     << duration.count() << " (sec)" << std::endl;
  os << "Samples          : " << stats.samples_so_far_ << std::endl;
  os << "Ray-object tests : " << stats.object_tests_ << std::endl;
  os << "Ray-object hits  : " << stats.object_hits_ << std::endl;
  os << "Hit rate         : " << std::fixed << std::setprecision(2)
     << (stats.object_hits_ / static_cast<double>(stats.object_tests_)) << "%"
     << std::endl;
  os << "----------------------------------->>" << std::endl;
  return os;
}

} // namespace muon