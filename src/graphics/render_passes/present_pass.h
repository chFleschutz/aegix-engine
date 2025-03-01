#pragma once

#include "graphics/frame_graph/frame_graph_render_pass.h"
#include "graphics/vulkan_tools.h"

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
				FrameGraphResourceUsage::TransferSrc
				});

			return FrameGraphNodeCreateInfo{
				.name = "Present",
				.inputs = { m_final },
				.outputs = {}
			};
		}

		virtual void execute(FrameGraphResourcePool& resources, const FrameInfo& frameInfo) override
		{
			Texture& srcTexture = resources.texture(m_final);
			assert(m_swapChain.width() == srcTexture.width() && "Swapchain extent does not match source texture extent");
			assert(m_swapChain.height() == srcTexture.height() && "Swapchain extent does not match source texture extent");
			
			VkCommandBuffer cmd = frameInfo.commandBuffer;
			VkImage srcImage = srcTexture.image();
			VkImage dstImage = m_swapChain.currentImage();
			
			Tools::vk::cmdPipelineBarrier(cmd, dstImage, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				VK_IMAGE_ASPECT_COLOR_BIT);

			VkOffset3D extent = { static_cast<int32_t>(m_swapChain.width()), static_cast<int32_t>(m_swapChain.height()), 1 };
			VkImageBlit blitRegion{};
			blitRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			blitRegion.srcSubresource.mipLevel = 0;
			blitRegion.srcSubresource.baseArrayLayer = 0;
			blitRegion.srcSubresource.layerCount = 1;
			blitRegion.srcOffsets[0] = { 0, 0, 0 };
			blitRegion.srcOffsets[1] = extent;
			blitRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			blitRegion.dstSubresource.mipLevel = 0;
			blitRegion.dstSubresource.baseArrayLayer = 0;
			blitRegion.dstSubresource.layerCount = 1;
			blitRegion.dstOffsets[0] = { 0, 0, 0 };
			blitRegion.dstOffsets[1] = extent;

			vkCmdBlitImage(cmd, 
				srcImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				dstImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				1, &blitRegion, VK_FILTER_LINEAR);

			Tools::vk::cmdPipelineBarrier(cmd, dstImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_IMAGE_ASPECT_COLOR_BIT);
		}

	private:
		SwapChain& m_swapChain;
		FrameGraphResourceHandle m_final;
		FrameGraphResourceHandle m_presentImage;
	};
}