#include "swap_chain_blit_pass.h"

#include <cassert>

namespace Aegix::Graphics
{
	SwapChainBlitPass::SwapChainBlitPass(Builder& builder, SwapChain& swapChain)
		: RenderPass(builder), m_swapChain{ swapChain }
	{
	}

	void SwapChainBlitPass::execute(FrameInfo& frameInfo)
	{
		// Bypass the begin- and endRenderPass (image blit doesn't need a render pass)

		render(frameInfo);
	}

	void SwapChainBlitPass::render(FrameInfo& frameInfo)
	{
		auto width = static_cast<int32_t>(m_renderArea.width);
		auto height = static_cast<int32_t>(m_renderArea.height);
		auto swapChainWidth = static_cast<int32_t>(m_swapChain.width());
		auto swapChainHeight = static_cast<int32_t>(m_swapChain.height());

		assert(width == swapChainWidth && height == swapChainHeight && "Render area and swap chain dimensions must match");
		assert(m_colorAttachments.size() == 1 && "SwapChainBlitPass must have exactly one color attachment");
		assert(m_colorAttachments[0].image && "Color attachment image not initialized");

		VkImage srcImage = m_colorAttachments[0].image->image();
		VkImage dstImage = m_swapChain.image(frameInfo.frameIndex);

		// Transition image layouts for blit
		m_device.transitionImageLayout(frameInfo.commandBuffer, 
			srcImage, 
			VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, 
			VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

		m_device.transitionImageLayout(frameInfo.commandBuffer,
			dstImage,
			VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);


		// Blit image to swap chain 
		VkImageBlit blit{};
		blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		blit.srcSubresource.layerCount = 1;
		blit.srcOffsets[0] = { 0, 0, 0 };
		blit.srcOffsets[1] = { width, height, 1 };
		blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		blit.dstSubresource.layerCount = 1;
		blit.dstOffsets[0] = { 0, 0, 0 };
		blit.dstOffsets[1] = { swapChainWidth, swapChainHeight, 1 };

		vkCmdBlitImage(frameInfo.commandBuffer,
			srcImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			dstImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1, &blit,
			VK_FILTER_LINEAR);


		// Transition image layouts back 
		m_device.transitionImageLayout(frameInfo.commandBuffer,
			srcImage,
			VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

		m_device.transitionImageLayout(frameInfo.commandBuffer,
			dstImage,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
	}
}