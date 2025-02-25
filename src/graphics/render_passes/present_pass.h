#pragma once

#include "graphics/frame_graph/frame_graph_render_pass.h"

namespace Aegix::Graphics
{
	class PresentPass : public FrameGraphRenderPass
	{
	public:
		PresentPass(SwapChain& swapChain)
			: m_swapChain{ swapChain }
		{
		}

		virtual auto createInfo(FrameGraphResourceBuilder& builder) -> FrameGraphNodeCreateInfo override
		{
			m_final = builder.add({ "Final",
				FrameGraphResourceType::Reference,
				FrameGraphResourceUsage::Present
				});

			m_presentImage = builder.add(
				Texture{ m_swapChain.device(), m_swapChain },
				FrameGraphResourceCreateInfo{ 
					.name = "SwapChainImage",
					.type = FrameGraphResourceType::Texture,
					.usage = FrameGraphResourceUsage::Present,
					.info = FrameGraphResourceTextureInfo{
						.format = VK_FORMAT_B8G8R8A8_SRGB,
						.extent = { 0, 0 },
						.resizePolicy = ResizePolicy::Fixed
					}
				});

			return FrameGraphNodeCreateInfo{
				.name = "Present Pass",
				.inputs = { m_final },
				.outputs = { m_presentImage }
			};
		}

		virtual void prepare(FrameGraphResourcePool& resources, const FrameInfo& frameInfo) override
		{
			resources.texture(m_presentImage).update(m_swapChain);
		}

		virtual void execute(FrameGraphResourcePool& resources, const FrameInfo& frameInfo) override
		{
			resources.texture(m_presentImage).update(VK_NULL_HANDLE, VK_NULL_HANDLE);
		}

	private:
		SwapChain& m_swapChain;
		FrameGraphResourceHandle m_final;
		FrameGraphResourceHandle m_presentImage;
	};
}