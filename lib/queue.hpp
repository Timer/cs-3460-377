#ifndef CON_QUEUE_HPP
#define CON_QUEUE_HPP

#include <list>
#include <mutex>
#include <condition_variable>

template <typename T>
class concurrent_queue
{
private:
	std::list<T> list;
	std::mutex mtx;
	std::condition_variable cv;
public:
	concurrent_queue() {
	}

	concurrent_queue(concurrent_queue &&) = delete;
	concurrent_queue(const concurrent_queue &) = delete;

	concurrent_queue &operator =(concurrent_queue &&) = delete;
	concurrent_queue &operator =(const concurrent_queue &&) = delete;

	void push(T &&t) {
		std::unique_lock<std::mutex> lock(mtx);
		list.push_back(std::move(t));
		cv.notify_all();
	}
	void push(const T &t) {
		std::unique_lock<std::mutex> lock(mtx);
		list.push_back(t);
		cv.notify_all();
	}

	T pop() {
		std::unique_lock<std::mutex> lock(mtx);
		while (list.empty()) cv.wait(lock);
				auto t = std::move(list.front());
		list.pop_front();
		return t;

	}
};

#endif
