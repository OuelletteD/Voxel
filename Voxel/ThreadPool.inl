#pragma once

template<typename F>
std::future<void> ThreadPool::enqueue(F&& task){
	auto packagedTask = std::make_shared<std::packaged_task<void()>>(std::forward<F>(task));
	std::future<void> result = packagedTask->get_future();
	{
		std::unique_lock<std::mutex> lock(queueMutex);
		tasks.emplace([packagedTask]() { (*packagedTask)(); });
	}
	condition.notify_one();
	return result;
}