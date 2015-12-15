#ifndef POOL_HPP
#define POOL_HPP

#include <atomic>
#include <deque>
#include <future>
#include <mutex>
#include <thread>
#include <vector>

//TODO: reject submit when pool is shutdown
class Pool {
private:
  //Count of threads in the pool
  int count;
  //Mutex when performing actions on the non-thread-safe STL utilities
  std::mutex m;
  //Stuff for putting threads to sleepy sleepy
  std::mutex m2;
  std::condition_variable cv;
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
        //No work, go (back) to sleep!
        m.unlock();

		std::unique_lock<std::mutex> lock(m2);
		cv.wait(lock);
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
	{
		std::unique_lock<std::mutex> lock(m2);
		cv.notify_all();
	}
    //Wait for all workers to finish current work
    for (auto i = threads.begin(); i != threads.end(); ++i) i->join();
    //Clean up our atomic variable
    delete shutdown;
  }

  template <typename Iter, typename Fn>
  void parallel_for(Iter start, Iter end, Fn fn) {
    const int THREADS = size();
    const int COUNT = end - start;
    auto block_size = COUNT / size();
    if (block_size < 1) block_size = 1;
    std::vector<std::future<void>> futures;
    futures.reserve(THREADS);
    auto curr = start;
    for (auto i = 0; i < THREADS && curr < end; ++i) {
      auto next = curr + block_size;
      if (next >= end) {
        next = end;
      }
      futures.push_back(submit([curr, next, fn]() {
        for (auto j = curr; j != next; ++j) fn(j);
      }));
      curr = next;
    }
    for (auto it = futures.begin(); it != futures.end(); ++it) it->wait();
  }

  int size() const { return this->count; }

  template <typename F>
  auto submit(F&& f) -> std::future<decltype(f())> {
    auto p = std::make_shared<std::packaged_task<decltype(f())()>>(std::forward<F>(f));
    m.lock();
    auto l = [p]() {
      (*p)();
    };
    jobs.emplace_back(std::async(std::launch::deferred, std::move(l)));
    m.unlock();
	std::unique_lock<std::mutex> lock(m2);
	cv.notify_one();
    return p->get_future();
  }

  template <typename F, typename... A>
  auto submit(F&& f, A&&... a) -> std::future<decltype(f(a...))> {
    auto p = std::make_shared<std::packaged_task<decltype(f(a...))()>>(std::bind(std::forward<F>(f), std::forward<A>(a)...));
    m.lock();
    auto l = [p]() {
      (*p)();
    };
    jobs.emplace_back(std::async(std::launch::deferred, std::move(l)));
    m.unlock();
	std::unique_lock<std::mutex> lock(m2);
	cv.notify_one();
    return p->get_future();
  }
};

#endif
