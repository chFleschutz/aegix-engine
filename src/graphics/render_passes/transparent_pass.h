#pragma once

#include "graphics/frame_graph/frame_graph_render_pass.h"

namespace Aegix::Graphics
{
	class TransparentPass : public FrameGraphRenderPass
	{
	public:
		TransparentPass()
		{
		}

		virtual auto createInfo(FrameGraphResourceBuilder& builder) -> FrameGraphNodeCreateInfo override
		{
			m_sceneColor = builder.add({ "SceneColor",
				FrameGraphResourceType::Reference,
				FrameGraphResourceUsage::ColorAttachment
				});
			m_depth = builder.add({ "Depth",
				FrameGraphResourceType::Reference,
				FrameGraphResourceUsage::DepthStencilAttachment
				});

			return FrameGraphNodeCreateInfo{
				.name = "Transparent Pass",
				.inputs = { m_sceneColor, m_depth },
				.outputs = { m_sceneColor }
			};
		}

		virtual void execute(FrameGraphResourcePool& resources, const FrameInfo& frameInfo) override
		{
			// Begin rendering

			// Draw transparent objects

			// End rendering
		}

	private:
		FrameGraphResourceHandle m_sceneColor;
		FrameGraphResourceHandle m_depth;
	};
}