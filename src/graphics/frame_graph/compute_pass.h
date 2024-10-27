#pragma once

#include "graphics/frame_graph/frame_graph_render_pass.h"

namespace Aegix::Graphics
{
	class ComputePass : public FrameGraphPass
	{
	public:
		ComputePass(VulkanDevice& device) : FrameGraphPass{ device } {}

		virtual void execute(const FrameInfo& frameInfo) override
		{
			FrameGraphPass::execute(frameInfo);
			dispatchCompute();
		}

	private:
		void dispatchCompute() 
		{
			// TODO 
		}
	};
}