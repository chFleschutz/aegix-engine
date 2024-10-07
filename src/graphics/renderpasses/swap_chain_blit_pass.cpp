#include "swap_chain_blit_pass.h"

namespace Aegix::Graphics
{
	SwapChainBlitPass::SwapChainBlitPass(Builder& builder, SwapChain& swapChain)
		: RenderPass(builder), m_swapChain{ swapChain }
	{
	}

	void SwapChainBlitPass::render(FrameInfo& frameInfo)
	{
		auto width = static_cast<int32_t>(m_renderArea.width);
		auto height = static_cast<int32_t>(m_renderArea.height);
		auto swapChainWidth = static_cast<int32_t>(m_swapChain.width());
		auto swapChainHeight = static_cast<int32_t>(m_swapChain.height());

		// TODO: transition src and swapchain images to correct layout
		// TODO: get correct src image

		VkImageBlit blit{};
		blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		blit.srcSubresource.mipLevel = 0;
		blit.srcSubresource.baseArrayLayer = 0;
		blit.srcSubresource.layerCount = 1;
		blit.srcOffsets[0] = { 0, 0, 0 };
		blit.srcOffsets[1] = { width, height, 1 };
		blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		blit.dstSubresource.mipLevel = 0;
		blit.dstSubresource.baseArrayLayer = 0;
		blit.dstSubresource.layerCount = 1;
		blit.dstOffsets[0] = { 0, 0, 0 };
		blit.dstOffsets[1] = { swapChainWidth, swapChainHeight, 1 };

		vkCmdBlitImage(frameInfo.commandBuffer,
			nullptr, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			nullptr, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1, &blit,
			VK_FILTER_LINEAR
		);
	}
}