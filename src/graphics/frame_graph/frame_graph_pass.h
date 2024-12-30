#pragma once

#include <functional>

namespace Aegix::Graphics
{
	struct FrameGraphPassBase
	{
		virtual ~FrameGraphPassBase() = default;
		virtual void operator()() = 0;
	};

	template <typename Data, typename Execute>
	struct FrameGraphPass : public FrameGraphPassBase
	{
		explicit FrameGraphPass(Execute&& exec)
			: execute{ std::forward<Execute>(exec) }
		{
		}

		void operator()() override
		{
			execute(data);
		}

		Data data{};
		Execute execute;
	};
}