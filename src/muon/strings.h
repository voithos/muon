#ifndef MUON_STRINGS_H_
#define MUON_STRINGS_H_

#include <algorithm>
#include <string>

namespace muon {

// Trims from start (in place).
static inline void ltrim(std::string &s) {
  s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
            return !std::isspace(ch);
          }));
}

// Trims from end (in place).
static inline void rtrim(std::string &s) {
  s.erase(std::find_if(s.rbegin(), s.rend(),
                       [](unsigned char ch) { return !std::isspace(ch); })
              .base(),
          s.end());
}

// Trims from both ends (in place).
static inline void trim(std::string &s) {
  ltrim(s);
  rtrim(s);
}

// Trims from start (copying).
static inline std::string ltrim_copy(std::string s) {
  ltrim(s);
  return s;
}

// Trims from end (copying).
static inline std::string rtrim_copy(std::string s) {
  rtrim(s);
  return s;
}

// Trims from both ends (copying).
static inline std::string trim_copy(std::string s) {
  trim(s);
  return s;
}

} // namespace muon

#endif
