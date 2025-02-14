#pragma once

#include "graphics/frame_info.h"

#include <functional>

namespace Aegix::Graphics
{
	struct FrameGraphPassBase
	{
		virtual ~FrameGraphPassBase() = default;
		virtual void operator()(const FrameInfo& frameInfo) = 0;
	};

	template <typename Data, typename Execute>
	struct FrameGraphPass : public FrameGraphPassBase
	{
		explicit FrameGraphPass(Execute&& exec)
			: executeFunc{ std::forward<Execute>(exec) }
		{
		}

		void operator()(const FrameInfo& frameInfo) override
		{
			executeFunc(data, frameInfo);
		}

		Data data{};
		Execute executeFunc;
	};
}