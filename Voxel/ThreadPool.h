#pragma once
#include <future>
#include <queue>
#include <functional>
#include <atomic>
#include <condition_variable>
#include <mutex>
#include <thread>

class ThreadPool{
public: 
	explicit ThreadPool(size_t threadCount = std::thread::hardware_concurrency());
	~ThreadPool();
	template<typename F>
	std::future<void> enqueue(F&& tasK);
	int GetPendingTaskCount() {
		std::unique_lock<std::mutex> lock(queueMutex);
		return static_cast<int>(tasks.size());
	}
private:
	std::vector<std::thread> workers;
	std::queue<std::function<void()>> tasks;
	std::mutex queueMutex;
	std::condition_variable condition;
	std::atomic<bool> stop;

	void workerLoop();
};

#include "ThreadPool.inl"
