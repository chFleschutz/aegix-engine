#pragma once

#include "utils/rolling_average.h"
#include "utils/timer.h"

#include <chrono>
#include <string_view>
#include <unordered_map>

#define AGX_PROFILE_SCOPE(name) Aegix::ScopeProfiler profiler##__LINE__(name)
#define AGX_PROFILE_FUNCTION() AGX_PROFILE_SCOPE(__FUNCTION__)

namespace Aegix
{
	class Profiler
	{
	public:
		static constexpr int AVERAGE_FRAME_COUNT = 50;

		Profiler() = default;
		~Profiler() = default;

		static Profiler& instance()
		{
			static Profiler instance;
			return instance;
		}

		[[nodicard]] auto time(const std::string& name) const -> double
		{
			auto it = m_times.find(name);
			if (it == m_times.end())
				return 0.0;
			return it->second.average();
		}

		void addTime(const std::string& name, double time)
		{
			m_times[name].add(time);
		}

		auto times() const -> const std::unordered_map<std::string, RollingAverage<AVERAGE_FRAME_COUNT>>&
		{
			return m_times;
		}

	private:
		std::unordered_map<std::string, RollingAverage<AVERAGE_FRAME_COUNT>> m_times;
	};

	class ScopeProfiler
	{
	public:
		ScopeProfiler(std::string_view name)
			: m_name(name)
		{
		}

		~ScopeProfiler()
		{
			Profiler::instance().addTime(m_name, m_timer.elapsedMillis());
		}

	private:
		Timer m_timer;
		std::string m_name;
	};
}