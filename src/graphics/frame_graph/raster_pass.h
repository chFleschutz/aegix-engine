#pragma once

#include "graphics/frame_graph/frame_graph_render_pass.h"

namespace Aegix::Graphics
{
	class RasterPass : public FrameGraphRenderPass
	{
	public:

	private:
		void createRenderPass();
		void createFramebuffer();

		VkRenderPass m_renderPass;
		VkFramebuffer m_framebuffer;
	};
}
