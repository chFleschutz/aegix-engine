#pragma once

#include "graphics/frame_graph/frame_graph_render_pass.h"

namespace Aegix::Graphics
{
	class PresentPass : public FrameGraphRenderPass
	{
	public:
		PresentPass()
		{
		}

		virtual auto createInfo(FrameGraphResourceBuilder& builder) -> FrameGraphNodeCreateInfo override
		{
			m_sceneColor = builder.add({ "Final",
				FrameGraphResourceType::Reference,
				FrameGraphResourceUsage::Present
				});

			return FrameGraphNodeCreateInfo{
				.name = "Present Pass",
				.inputs = { m_sceneColor },
				.outputs = {}
			};
		}

		virtual void execute(FrameGraphResourcePool& resources, const FrameInfo& frameInfo) override
		{
		}

	private:
		FrameGraphResourceHandle m_sceneColor;
	};
}