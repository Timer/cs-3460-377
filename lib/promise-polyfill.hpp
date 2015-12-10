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

template <class Body>
void task_loop(std::shared_ptr<std::promise<void>> p, Body body) {
  auto s = body().share();
  then(s, [=](auto f) {
    try {
      if (f.get())
        task_loop(p, body);
      else
        p->set_value();
    } catch (...) {
    }
  });
}

template <class Body>
std::future<void> do_while(Body body) {
  auto p = std::make_shared<std::promise<void>>();
  std::async(std::launch::async, [=]() {
    try {
      task_loop(p, body);
    } catch (...) {
    }
  });
  return p->get_future();
}
};

#endif
