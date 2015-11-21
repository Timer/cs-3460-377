#ifndef PROMISE_POLYFILL_HPP
#define PROMISE_POLYFILL_HPP

#include "pool.hpp"
#include <future>
#include <cstdio>

namespace Promise {
template <typename T, typename F>
auto then(std::shared_future<T> fut, F f) -> std::future<decltype(f(fut.get()))> {
  return std::async(std::launch::async, [fut, f]() {
    try {
      auto v = fut.get();
      return f(v);
    } catch (...) {
      throw std::current_exception();
    }
  });
}
};

#endif
