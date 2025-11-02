#pragma once

#include "FXLib.h"

#include <vector>
#include <thread>
#include <queue>
#include <functional>
#include <condition_variable>
#include <future>
#include <type_traits>

namespace FX
{
	class FXLIB_API ThreadPool
	{
	public:
		ThreadPool(size_t numThreads);
		~ThreadPool();

		template<class F, class... Args>
		auto enqueue(F&& f, Args&&... args) -> std::future<typename std::invoke_result<F, Args...>::type>;

	private:
		std::vector<std::thread> workers;
		std::queue<std::function<void()>> tasks;

		std::mutex queueMutex;
		std::condition_variable condition;
		bool stop;
	};

	template<class F, class... Args>
	auto ThreadPool::enqueue(F&& f, Args&&... args)->std::future<typename std::invoke_result<F, Args...>::type>
	{
		using returnType = typename std::invoke_result<F, Args...>::type;

		auto task = std::make_shared<std::packaged_task<returnType()>>(std::bind(std::forward<F>(f), std::forward<Args>(args)...));

		std::future<returnType> res = task->get_future();
		{
			std::unique_lock<std::mutex> lock(queueMutex);

			if (stop)
				throw std::runtime_error("erm");

			tasks.emplace([task]() { (*task)(); });
		}
		condition.notify_one();
		return res;
	}
}
