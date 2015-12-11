#ifndef TIME_HPP
#define TIME_HPP

#include <chrono>

inline std::chrono::time_point<std::chrono::high_resolution_clock> now() {
  return std::chrono::high_resolution_clock::now();
}

inline int to_seconds(std::chrono::time_point<std::chrono::high_resolution_clock> t1, std::chrono::time_point<std::chrono::high_resolution_clock> t2) {
  std::chrono::duration<double> diff = t2 - t1;
  return diff.count();
}

inline int to_milliseconds(std::chrono::time_point<std::chrono::high_resolution_clock> t1, std::chrono::time_point<std::chrono::high_resolution_clock> t2) {
  return std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
}

#endif
