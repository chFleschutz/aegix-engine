#pragma once

#include "graphics/frame_graph/frame_graph_render_pass.h"

namespace Aegix::Graphics
{
	class GUIPass : public FrameGraphRenderPass
	{
	public:
		GUIPass()
		{

		}

		virtual auto createInfo(FrameGraphResourceBuilder& builder) -> FrameGraphNodeCreateInfo override
		{
			m_final = builder.add({
				"Final", 
				FrameGraphResourceType::Reference,
				FrameGraphResourceUsage::None
				});

			return FrameGraphNodeCreateInfo{
				.name = "GUI Pass",
				.inputs = { m_final },
				.outputs = { m_final }
			};
		}

		virtual void execute(FrameGraphResourcePool& resources, const FrameInfo& frameInfo) override
		{
		}

	private:
		FrameGraphResourceHandle m_final;
	};
}