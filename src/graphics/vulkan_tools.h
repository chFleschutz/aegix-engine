#pragma once

#include "graphics/texture.h"

#include "vulkan/vulkan.h"

#include <cassert>
#include <iostream>
#include <string_view>
#include <vector>

#define VK_CHECK(f)																						\
{																										\
	VkResult result = (f);																				\
	if (result != VK_SUCCESS)																			\
	{																									\
		std::cout << "Vulkan Error: " << Aegix::Tools::resultString(result) << " at " << __FILE__ << ", line " << __LINE__ << "\n"; \
		assert(result == VK_SUCCESS);																	\
	}																									\
}

namespace Aegix::Tools
{
	/// @brief Returns the result as a string
	std::string_view resultString(VkResult result);

	/// @brief Returns true if the format is a depth or depth-stencil format otherwise false
	bool isDepthFormat(VkFormat format);

	auto srcAccessMask(VkImageLayout layout) -> VkAccessFlags;
	auto dstAccessMask(VkImageLayout layout) -> VkAccessFlags;

	auto srcStage(VkAccessFlags access) -> VkPipelineStageFlags;
	auto dstStage(VkAccessFlags access) -> VkPipelineStageFlags;

	auto aspectFlags(VkFormat format) -> VkImageAspectFlags;

	auto renderingAttachmentInfo(VkImageView imageView, VkImageLayout layout, VkAttachmentLoadOp loadOp, VkClearValue clearValue) -> VkRenderingAttachmentInfo;
	auto renderingAttachmentInfo(const Graphics::Texture& texture, VkAttachmentLoadOp loadOp, VkClearValue clearValue) -> VkRenderingAttachmentInfo;


	namespace vk
	{
		void cmdPipelineBarrier(VkCommandBuffer commandBuffer, VkPipelineStageFlags srcStage, VkPipelineStageFlags dstStage,
			const std::vector<VkImageMemoryBarrier>& barriers);

		void cmdViewport(VkCommandBuffer commandBuffer, VkExtent2D extent);

		void cmdScissor(VkCommandBuffer commandBuffer, VkExtent2D extent);

		void cmdBindDescriptorSet(VkCommandBuffer commandBuffer, VkPipelineBindPoint bindPoint, VkPipelineLayout layout, VkDescriptorSet descriptorSet);
	}
}
