#pragma once

#include "core/logging.h"
#include "graphics/resources/texture.h"

#include <vulkan/vulkan.h>

#define VK_CHECK(f)																						\
{																										\
	VkResult result = (f);																				\
	if (result != VK_SUCCESS)																			\
	{																									\
		ALOG::fatal("Vulkan Error: {} in function '{}' at {}, line {}\nExpression: {}",					\
			Aegix::Tools::resultString(result), __FUNCTION__, __FILE__, __LINE__, #f);					\
		std::terminate();																				\
	}																									\
}

namespace Aegix::Tools
{
	void loadFunctionPointers(VkInstance instance);

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
	auto renderingAttachmentInfo(const Graphics::Texture& texture, VkAttachmentLoadOp loadOp, VkClearValue clearValue = { 0.0f, 0.0f, 0.0f, 0.0f }) -> VkRenderingAttachmentInfo;

	auto createShaderModule(VkDevice device, const std::vector<char>& code) -> VkShaderModule;
	auto createShaderModule(VkDevice device, const std::filesystem::path& path) -> VkShaderModule;

	auto createShaderStage(VkShaderStageFlagBits stage, VkShaderModule module) -> VkPipelineShaderStageCreateInfo;

	namespace vk
	{
		void cmdBindDescriptorSet(VkCommandBuffer commandBuffer, VkPipelineBindPoint bindPoint, VkPipelineLayout layout, VkDescriptorSet descriptorSet, uint32_t firstSet = 0);

		void cmdCopyBufferToImage(VkCommandBuffer cmd, VkBuffer buffer, VkImage image, VkExtent2D extent);
		void cmdCopyBufferToImage(VkCommandBuffer cmd, VkBuffer buffer, VkImage image, VkExtent3D extent, uint32_t layerCount);

		void cmdDispatch(VkCommandBuffer cmd, VkExtent2D extent, VkExtent2D groupSize);
		void cmdDispatch(VkCommandBuffer cmd, VkExtent3D extent, VkExtent3D groupSize);

		void cmdPipelineBarrier(VkCommandBuffer cmd, VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout,
			VkImageAspectFlags aspectMask);

		void cmdPipelineBarrier(VkCommandBuffer commandBuffer, VkPipelineStageFlags srcStage, VkPipelineStageFlags dstStage,
			const std::vector<VkImageMemoryBarrier>& barriers);

		template<typename T>
		void cmdPushConstants(VkCommandBuffer cmd, VkPipelineLayout layout, VkShaderStageFlags stage, const T& data)
		{
			vkCmdPushConstants(cmd, layout, stage, 0, sizeof(T), &data);
		}

		void cmdScissor(VkCommandBuffer commandBuffer, VkExtent2D extent);

		void cmdTransitionImageLayout(VkCommandBuffer cmd, VkImage image, VkFormat format, VkImageLayout oldLayout,
			VkImageLayout newLayout, uint32_t miplevels = 1, uint32_t layoutCount = 1);

		void cmdViewport(VkCommandBuffer commandBuffer, VkExtent2D extent);


		// Extensions

		void cmdBeginDebugUtilsLabel(VkCommandBuffer cmd, const char* label, const glm::vec4& color = glm::vec4{ 1.0f });

		void cmdEndDebugUtilsLabel(VkCommandBuffer cmd);
	}
}
