#ifndef TIME_HPP
#define TIME_HPP

#include <chrono>

inline auto now() {
	return std::chrono::high_resolution_clock::now();
}

inline auto to_seconds(std::chrono::time_point<std::chrono::high_resolution_clock> t1, std::chrono::time_point<std::chrono::high_resolution_clock> t2) {
	std::chrono::duration<double> diff = t2 - t1;
	return diff.count();
}

#endif
