#ifndef POOL_HPP
#define POOL_HPP

#include <atomic>
#include <deque>
#include <future>
#include <mutex>
#include <thread>
#include <vector>

class Pool {
private:
  //Count of threads in the pool
  int count;
  //Mutex when performing actions on the non-thread-safe STL utilities
  std::mutex m;
  //List of our workers
  std::vector<std::thread> threads;
  //Whether or not we're accepting tasks
  std::atomic<bool>* shutdown = new std::atomic<bool>(false);
  //List of pending jobs waiting to be std::move'd by a worker
  std::deque<std::future<void>> jobs;

  void provision() {
    //Lock while we create our threads (do you really think someone will try deleting us that fast? :sad_face:)
    m.lock();
    for (int i = 0; i < count; ++i) {
      //Create a thread bound to the object which owns this pool object (because why not?)
      threads.push_back(std::move(std::thread(&Pool::create_worker, this)));
    }
    m.unlock();
  }

  void create_worker() {
    for (;;) {
      //Die if we're shutting down
      if (shutdown->load()) break;
      //Lock the mutex and check for work
      m.lock();
      if (jobs.empty()) {
        //No work, go back to sleep!
        m.unlock();
        //TODO: condition variable
        std::this_thread::yield();
      } else {
        //Ermahgerd we have work! Leggo.
        auto f = std::move(jobs.front());
        jobs.pop_front();
        m.unlock();

        f.get();
      }
    }
  }

public:
  Pool() : Pool(std::thread::hardware_concurrency() * 3) {}

  Pool(const int& count) {
    this->count = count;
    provision();
  }

  ~Pool() {
    //Signal shutdown to the workers
    shutdown->store(true);
    //Wait for all workers to finish current work
    for (auto i = threads.begin(); i != threads.end(); ++i) i->join();
    //Clean up our atomic variable
    delete shutdown;
  }

  int size() const { return this->count; }

  template <typename F>
  auto submit(F&& f) -> std::future<decltype(f())> {
    typedef decltype(f()) R;
    auto p = std::make_shared<std::packaged_task<R()>>(std::forward<F>(f));
    m.lock();
    auto l = [p]() {
      (*p)();
    };
    jobs.emplace_back(std::async(std::launch::deferred, std::move(l)));
    m.unlock();
    return p->get_future();
  }

  template <typename F, typename... A>
  auto submit(F&& f, A&&... a) -> std::future<decltype(f(a...))> {
    typedef decltype(f(a...)) R;
    auto p = std::make_shared<std::packaged_task<R()>>(std::bind(std::forward<F>(f), std::forward<A>(a)...));
    m.lock();
    auto l = [p]() {
      (*p)();
    };
    jobs.emplace_back(std::async(std::launch::deferred, std::move(l)));
    m.unlock();
    return p->get_future();
  }
};

#endif
