#pragma once

#include <functional>
#include <queue>
#include <mutex>

class MainThreadDispatcher {
public:
	void Enqueue(std::function<void()> task) {
		std::lock_guard<std::mutex> lock(mutex);
		tasks.push(std::move(task));
	}
	void Process() {
		std::queue<std::function<void()>> tasksCopy;
		{
			std::lock_guard<std::mutex> lock(mutex);
			std::swap(tasks, tasksCopy);
		}
		while (!tasksCopy.empty()) {
			tasksCopy.front()();
			tasksCopy.pop();
		}
	}
private:
	std::queue<std::function<void()>> tasks;
	std::mutex mutex;
};