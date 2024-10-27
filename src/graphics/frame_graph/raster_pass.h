#pragma once

#include "graphics/frame_graph/frame_graph_render_pass.h"

namespace Aegix::Graphics
{
	class RasterPass : public FrameGraphPass
	{
	public:
		RasterPass(VulkanDevice& device) : FrameGraphPass{ device } {}

		virtual void execute(const FrameInfo& frameInfo) override
		{
			beginRenderPass();
			FrameGraphPass::execute(frameInfo);
			endRenderPass();
		}

	private:
		void createRenderPass();
		void createFramebuffer();

		void beginRenderPass();
		void endRenderPass();

		VkRenderPass m_renderPass;
		VkFramebuffer m_framebuffer;
	};
}
