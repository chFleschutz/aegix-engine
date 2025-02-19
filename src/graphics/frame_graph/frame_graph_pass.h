#pragma once

#include "graphics/frame_graph/frame_graph_resource_pool.h"
#include "graphics/frame_info.h"

#include <functional>

namespace Aegix::Graphics
{
	struct FrameGraphPassBase
	{
		virtual ~FrameGraphPassBase() = default;
		virtual void operator()(FrameGraphResourcePool& resources, const FrameInfo& frameInfo) = 0;
	};

	template <typename Data, typename Execute>
	struct FrameGraphPass : public FrameGraphPassBase
	{
		explicit FrameGraphPass(Execute&& exec)
			: executeFunc{ std::forward<Execute>(exec) }
		{
		}

		void operator()(FrameGraphResourcePool& resources, const FrameInfo& frameInfo) override
		{
			executeFunc(data, resources, frameInfo);
		}

		Data data{};
		Execute executeFunc;
	};
}