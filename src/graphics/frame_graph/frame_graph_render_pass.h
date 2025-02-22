#pragma once

#include "graphics/frame_graph/frame_graph_resource_pool.h"

namespace Aegix::Graphics
{
	class FrameGraphRenderPass
	{
	public:
		FrameGraphRenderPass() = default;
		FrameGraphRenderPass(const FrameGraphRenderPass&) = delete;
		FrameGraphRenderPass(FrameGraphRenderPass&&) = delete;
		virtual ~FrameGraphRenderPass() = default;

		FrameGraphRenderPass& operator=(const FrameGraphRenderPass&) = delete;
		FrameGraphRenderPass& operator=(FrameGraphRenderPass&&) = delete;

		/// @brief Information required to create a FrameGraphNode (primarily for defining inputs and outputs)
		virtual auto createInfo(FrameGraphResourcePool& pool) -> FrameGraphNodeCreateInfo = 0;

		/// @brief Execute the render pass
		virtual void execute(FrameGraphResourcePool& resources, const FrameInfo& frameInfo) = 0;
	};
}