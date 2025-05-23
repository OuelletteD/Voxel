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
	void enqueue(F&& tasK);
private:
	std::vector<std::thread> workers;
	std::queue<std::function<void()>> tasks;
	std::mutex queueMutex;
	std::condition_variable condition;
	std::atomic<bool> stop;

	void workerLoop();
};

#include "ThreadPool.inl"
