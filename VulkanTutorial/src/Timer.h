#pragma once
#include <chrono>

namespace VulkanTutorial {

	class Timer
	{
	public:
		Timer();
		inline void reset() { m_StartTimePoint = std::chrono::high_resolution_clock::now(); }
		inline float ellapseSeconds() { return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - m_StartTimePoint).count() * 0.001f * 0.001f * 0.001f; }
		inline float ellapseMilliseconds() { return ellapseSeconds() * 1000.0f; }
	private:
		std::chrono::time_point<std::chrono::high_resolution_clock> m_StartTimePoint;
	};

}
