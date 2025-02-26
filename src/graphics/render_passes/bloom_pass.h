#pragma once

#include "graphics/frame_graph/frame_graph_render_pass.h"

namespace Aegix::Graphics
{
	class BloomPass : public FrameGraphRenderPass
	{
	public:
		BloomPass()
		{

		}

		virtual auto createInfo(FrameGraphResourceBuilder& builder) -> FrameGraphNodeCreateInfo override
		{
			m_sceneColor = builder.add({
				"SceneColor",
				FrameGraphResourceType::Reference,
				FrameGraphResourceUsage::Compute
				});

			m_bloom = builder.add({
				"Bloom",
				FrameGraphResourceType::Texture,
				FrameGraphResourceUsage::Compute,
				FrameGraphResourceTextureInfo{
					.format = VK_FORMAT_R16G16B16A16_SFLOAT,
					.extent = { 0, 0 },
					.resizePolicy = ResizePolicy::SwapchainRelative
					}
				});

			return FrameGraphNodeCreateInfo{
				.name = "Bloom",
				.inputs = { m_sceneColor },
				.outputs = { m_bloom },
			};
		}

		virtual void execute(FrameGraphResourcePool& resources, const FrameInfo& frameInfo) override
		{
			// Extract bright regions

			// Downsample

			// Upsample

			// Combine (in post processing pass)
		}

	private:
		FrameGraphResourceHandle m_sceneColor;
		FrameGraphResourceHandle m_bloom;
	};
}