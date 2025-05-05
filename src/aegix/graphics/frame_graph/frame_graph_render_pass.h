#pragma once

#include "graphics/frame_graph/frame_graph_resource_pool.h"
#include "graphics/frame_info.h"
#include "graphics/render_context.h"

// Included here to avoid issues in child classes
#include "scene/entity.h" 

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
		virtual auto createInfo(FrameGraphResourceBuilder& builder) -> FrameGraphNodeCreateInfo = 0;

		/// @brief Called when resource should be created (at startup, window resize)
		virtual void createResources(FrameGraphResourcePool& resources) {}

		/// @brief Execute the render pass
		virtual void execute(FrameGraphResourcePool& resources, const FrameInfo& frameInfo) = 0;

		/// @brief Draw the UI for the render pass in the renderer panel
		virtual void drawUI() {}
	};
}