#pragma once

#include "graphics/frame_graph/frame_graph_render_pass.h"

namespace Aegix::Graphics
{
	class PostProcessingPass : public FrameGraphRenderPass
	{
	public:
		PostProcessingPass()
		{
		}

		virtual auto createInfo(FrameGraphResourceBuilder& builder) -> FrameGraphNodeCreateInfo override
		{
			m_sceneColor = builder.add({
				"SceneColor",
				FrameGraphResourceType::Reference,
				FrameGraphResourceUsage::None
				});

			m_final = builder.add({
				"Final",
				FrameGraphResourceType::Texture,
				FrameGraphResourceUsage::None,
				FrameGraphResourceTextureInfo{
					.format = VK_FORMAT_B8G8R8A8_SRGB,
					.extent = { 0, 0},
					.resizePolicy = ResizePolicy::Fixed
					}
				});

			return FrameGraphNodeCreateInfo{
				.name = "Post Processing Pass",
				.inputs = { m_sceneColor },
				.outputs = { m_final }
			};
		}

		virtual void execute(FrameGraphResourcePool& resources, const FrameInfo& frameInfo) override
		{
		}

	private:
		FrameGraphResourceHandle m_sceneColor;
		FrameGraphResourceHandle m_final;
	};
}