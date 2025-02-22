#pragma once

#include "graphics/frame_graph/frame_graph_resource_pool.h"

namespace Aegix::Graphics
{
	class FrameGraphRenderPass
	{
	public:
		~FrameGraphRenderPass() = default;

		virtual auto createInfo() -> FrameGraphNodeCreateInfo = 0;
		virtual void execute(FrameGraphResourcePool& resources, const FrameInfo& frameInfo) = 0;
	};
}