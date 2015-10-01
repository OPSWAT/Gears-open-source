#pragma once

#include <thread>

class Timer
{
public:
	template <class callable, class... arguments>
	Timer(uint32_t interval, bool async, callable&& f, arguments&&... args)
	{
		std::function<typename std::result_of<callable(arguments...)>::type()>
			repeatTask(std::bind(std::forward<callable>(f), std::forward<arguments>(args)...));

		if (async)
		{
			std::thread([interval, repeatTask]() {
				while (repeatTask())
					std::this_thread::sleep_for(std::chrono::milliseconds(interval));
			}).detach();
		}
		else
		{
			while (repeatTask())	// repeat again if task returns true
				std::this_thread::sleep_for(std::chrono::milliseconds(interval));
		}
	}
};

