#pragma once

template<typename F>
void ThreadPool::enqueue(F&& task){
	{
		std::unique_lock<std::mutex> lock(queueMutex);
		tasks.emplace(std::forward<F>(task));
	}
	condition.notify_one();
}