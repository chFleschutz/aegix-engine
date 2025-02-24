#include "vulkan_tools.h"


namespace Aegix::Tools
{
	std::string_view resultString(VkResult result)
	{
		switch (result)
		{
#define CASE(name) case name: return #name
			CASE(VK_SUCCESS);
			CASE(VK_NOT_READY);
			CASE(VK_TIMEOUT);
			CASE(VK_EVENT_SET);
			CASE(VK_EVENT_RESET);
			CASE(VK_INCOMPLETE);
			CASE(VK_ERROR_OUT_OF_HOST_MEMORY);
			CASE(VK_ERROR_OUT_OF_DEVICE_MEMORY);
			CASE(VK_ERROR_INITIALIZATION_FAILED);
			CASE(VK_ERROR_DEVICE_LOST);
			CASE(VK_ERROR_MEMORY_MAP_FAILED);
			CASE(VK_ERROR_LAYER_NOT_PRESENT);
			CASE(VK_ERROR_EXTENSION_NOT_PRESENT);
			CASE(VK_ERROR_FEATURE_NOT_PRESENT);
			CASE(VK_ERROR_INCOMPATIBLE_DRIVER);
			CASE(VK_ERROR_TOO_MANY_OBJECTS);
			CASE(VK_ERROR_FORMAT_NOT_SUPPORTED);
			CASE(VK_ERROR_FRAGMENTED_POOL);
			CASE(VK_ERROR_UNKNOWN);
			CASE(VK_ERROR_OUT_OF_POOL_MEMORY);
			CASE(VK_ERROR_INVALID_EXTERNAL_HANDLE);
			CASE(VK_ERROR_FRAGMENTATION);
			CASE(VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS);
			CASE(VK_PIPELINE_COMPILE_REQUIRED);
			CASE(VK_ERROR_SURFACE_LOST_KHR);
			CASE(VK_ERROR_NATIVE_WINDOW_IN_USE_KHR);
			CASE(VK_SUBOPTIMAL_KHR);
			CASE(VK_ERROR_OUT_OF_DATE_KHR);
			CASE(VK_ERROR_INCOMPATIBLE_DISPLAY_KHR);
			CASE(VK_ERROR_VALIDATION_FAILED_EXT);
			CASE(VK_ERROR_INVALID_SHADER_NV);
			CASE(VK_ERROR_IMAGE_USAGE_NOT_SUPPORTED_KHR);
			CASE(VK_ERROR_VIDEO_PICTURE_LAYOUT_NOT_SUPPORTED_KHR);
			CASE(VK_ERROR_VIDEO_PROFILE_OPERATION_NOT_SUPPORTED_KHR);
			CASE(VK_ERROR_VIDEO_PROFILE_FORMAT_NOT_SUPPORTED_KHR);
			CASE(VK_ERROR_VIDEO_PROFILE_CODEC_NOT_SUPPORTED_KHR);
			CASE(VK_ERROR_VIDEO_STD_VERSION_NOT_SUPPORTED_KHR);
			CASE(VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT);
			CASE(VK_ERROR_NOT_PERMITTED_KHR);
			CASE(VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT);
			CASE(VK_THREAD_IDLE_KHR);
			CASE(VK_THREAD_DONE_KHR);
			CASE(VK_OPERATION_DEFERRED_KHR);
			CASE(VK_OPERATION_NOT_DEFERRED_KHR);
			CASE(VK_ERROR_INVALID_VIDEO_STD_PARAMETERS_KHR);
			CASE(VK_ERROR_COMPRESSION_EXHAUSTED_EXT);
			CASE(VK_INCOMPATIBLE_SHADER_BINARY_EXT);
#undef CASE
		default:
			return "RESULT_UNKNOWN";
		}
	}

	bool Aegix::Tools::isDepthFormat(VkFormat format)
	{
		switch (format)
		{
		case VK_FORMAT_D16_UNORM:
		case VK_FORMAT_X8_D24_UNORM_PACK32:
		case VK_FORMAT_D32_SFLOAT:
		case VK_FORMAT_D16_UNORM_S8_UINT:
		case VK_FORMAT_D24_UNORM_S8_UINT:
		case VK_FORMAT_D32_SFLOAT_S8_UINT:
			return true;
		default:
			return false;
		}
	}

	auto srcAccessMask(VkImageLayout layout) -> VkAccessFlags
	{
		switch (layout)
		{
		case VK_IMAGE_LAYOUT_UNDEFINED:
			return 0;
		case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
			return VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
			return VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
			return VK_ACCESS_SHADER_READ_BIT;
		case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
			return 0;
		case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
			return VK_ACCESS_TRANSFER_READ_BIT;
		case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
			return VK_ACCESS_TRANSFER_WRITE_BIT;
		default:
			assert(false && "Unsupported layout transition");
			return 0;
		}
	}

	auto dstAccessMask(VkImageLayout layout) -> VkAccessFlags
	{
		switch (layout)
		{
		case VK_IMAGE_LAYOUT_UNDEFINED:
			return 0;
		case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
			return VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
		case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
			return VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
		case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
			return VK_ACCESS_SHADER_READ_BIT;
		case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
			return 0;
		case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
			return VK_ACCESS_TRANSFER_READ_BIT;
		case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
			return VK_ACCESS_TRANSFER_WRITE_BIT;
		default:
			assert(false && "Unsupported layout transition");
			return 0;
		}
	}

	auto srcStage(VkAccessFlags access) -> VkPipelineStageFlags
	{
		if (access & VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT)
			return VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		if (access & VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT)
			return VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
		if (access & VK_ACCESS_SHADER_READ_BIT)
			return VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		if (access & VK_ACCESS_SHADER_WRITE_BIT)
			return VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
		if (access & VK_ACCESS_TRANSFER_READ_BIT)
			return VK_PIPELINE_STAGE_TRANSFER_BIT;
		if (access & VK_ACCESS_TRANSFER_WRITE_BIT)
			return VK_PIPELINE_STAGE_TRANSFER_BIT;
		
		return VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
	}

	auto dstStage(VkAccessFlags access) -> VkPipelineStageFlags
	{
		if (access & VK_ACCESS_COLOR_ATTACHMENT_READ_BIT)
			return VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		if (access & VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT)
			return VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		if (access & VK_ACCESS_SHADER_READ_BIT)
			return VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		if (access & VK_ACCESS_SHADER_WRITE_BIT)
			return VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
		if (access & VK_ACCESS_TRANSFER_READ_BIT)
			return VK_PIPELINE_STAGE_TRANSFER_BIT;
		if (access & VK_ACCESS_TRANSFER_WRITE_BIT)
			return VK_PIPELINE_STAGE_TRANSFER_BIT;

		return VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	}

	auto aspectFlags(VkFormat format) -> VkImageAspectFlags
	{
		switch (format)
		{
		case VK_FORMAT_D16_UNORM:
		case VK_FORMAT_X8_D24_UNORM_PACK32:
		case VK_FORMAT_D32_SFLOAT:
			return VK_IMAGE_ASPECT_DEPTH_BIT;
		case VK_FORMAT_S8_UINT:
			return VK_IMAGE_ASPECT_STENCIL_BIT;
		case VK_FORMAT_D16_UNORM_S8_UINT:
		case VK_FORMAT_D24_UNORM_S8_UINT:
		case VK_FORMAT_D32_SFLOAT_S8_UINT:
			return VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
		default:
			return VK_IMAGE_ASPECT_COLOR_BIT;  // For all color formats
		}
	}

	auto renderingAttachmentInfo(VkImageView imageView, VkImageLayout layout, VkAttachmentLoadOp loadOp, VkClearValue clearValue) -> VkRenderingAttachmentInfo
	{
		VkRenderingAttachmentInfo attachment{};
		attachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
		attachment.imageView = imageView;
		attachment.imageLayout = layout;
		attachment.loadOp = loadOp;
		attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachment.clearValue = clearValue;
		return attachment;
	}

	auto renderingAttachmentInfo(const Graphics::Texture& texture, VkAttachmentLoadOp loadOp, VkClearValue clearValue) -> VkRenderingAttachmentInfo
	{
		return renderingAttachmentInfo(texture.imageView(), texture.layout(), loadOp, clearValue);
	}

	void vk::cmdPipelineBarrier(VkCommandBuffer commandBuffer, VkPipelineStageFlags srcStage, VkPipelineStageFlags dstStage, const std::vector<VkImageMemoryBarrier>& barriers)
	{
		if (barriers.empty())
			return;

		vkCmdPipelineBarrier(commandBuffer,
			srcStage, dstStage,
			0,
			0, nullptr,
			0, nullptr,
			static_cast<uint32_t>(barriers.size()), barriers.data()
		);
	}

	void vk::cmdViewport(VkCommandBuffer commandBuffer, VkExtent2D extent)
	{
		VkViewport viewport{};
		viewport.width = static_cast<float>(extent.width);
		viewport.height = static_cast<float>(extent.height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
	}

	void vk::cmdScissor(VkCommandBuffer commandBuffer, VkExtent2D extent)
	{
		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = extent;
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
	}

	void vk::cmdBindDescriptorSet(VkCommandBuffer commandBuffer, VkPipelineBindPoint bindPoint, VkPipelineLayout layout, VkDescriptorSet descriptorSet)
	{
		vkCmdBindDescriptorSets(commandBuffer, bindPoint, layout, 0, 1, &descriptorSet, 0, nullptr);
	}
}
