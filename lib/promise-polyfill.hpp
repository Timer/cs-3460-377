#ifndef PROMISE_POLYFILL_HPP
#define PROMISE_POLYFILL_HPP

#include "pool.hpp"
#include <future>

namespace Promise {
template <typename T, typename F>
auto then(std::shared_future<T> fut, F &&f) -> std::future<decltype(f(fut))> {
  auto p = std::make_shared<std::packaged_task<decltype(f(fut))()>>(std::bind(std::forward<F>(f), fut));
  auto l = [p]() {
    (*p)();
  };
  std::async(std::launch::async, std::move(l));
  return p->get_future();
}
};

#endif
