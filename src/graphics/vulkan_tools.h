#pragma once

#include "vulkan/vulkan.h"

#include <cassert>
#include <iostream>
#include <string_view>
#include <vector>

#define VK_CHECK(f)																				\
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

	auto aspectFlags(VkFormat format) -> VkImageAspectFlags;

	namespace vk
	{
		void cmdPipelineBarrier(VkCommandBuffer commandBuffer, VkPipelineStageFlags srcStage, VkPipelineStageFlags dstStage,
			const std::vector<VkImageMemoryBarrier>& barriers);
	}
}
