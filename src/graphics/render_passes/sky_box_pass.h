#pragma once

#include "graphics/frame_graph/frame_graph_render_pass.h"

namespace Aegix::Graphics
{
	class SkyBoxPass : public FrameGraphRenderPass
	{
	public:
		SkyBoxPass()
		{

		}

		virtual auto createInfo(FrameGraphResourceBuilder& builder) -> FrameGraphNodeCreateInfo
		{
			m_sceneColor = builder.add(FrameGraphResourceCreateInfo{
				.name = "Scene Color",
				.type = FrameGraphResourceType::Reference,
				.usage = FrameGraphResourceUsage::ColorAttachment,
				});

			m_depth = builder.add(FrameGraphResourceCreateInfo{
				.name = "Depth",
				.type = FrameGraphResourceType::Reference,
				.usage = FrameGraphResourceUsage::DepthStencilAttachment,
				});

			return FrameGraphNodeCreateInfo{
				.name = "Sky Box",
				.inputs = { m_sceneColor, m_depth },
				.outputs = { m_sceneColor },
			};
		}

		virtual void execute(FrameGraphResourcePool& resources, const FrameInfo& frameInfo) override
		{

		}

	private:
		FrameGraphResourceHandle m_sceneColor;
		FrameGraphResourceHandle m_depth;
	};
}
