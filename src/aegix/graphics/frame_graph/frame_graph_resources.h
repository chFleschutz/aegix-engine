#pragma once

#include "graphics/vulkan/volk_include.h"

namespace Aegix::Graphics
{
	struct FGHandle
	{
		uint32_t handle{ std::numeric_limits<uint32_t>::max() };
		[[nodiscard]] auto isValid() const -> bool { return handle != std::numeric_limits<uint32_t>::max(); }
		[[nodiscard]] auto operator==(const FGHandle& other) const -> bool { return handle == other.handle; }
	};

	struct FGNodeHandle : FGHandle {};
	struct FGResourceHandle : FGHandle {};
	struct FGBufferHandle : FGHandle {};
	struct FGTextureHandle : FGHandle {};

	enum class FGResourceUsage
	{
		None,

		ColorAttachment,
		DepthStencilAttachment,
		FragmentReadSampled,

		ComputeReadStorage,
		ComputeWriteStorage,
		ComputeReadSampled,

		TransferSrc,
		TransferDst,
		Present,
	};

	enum class FGResizeMode
	{
		Fixed,
		SwapChainRelative,
	};

	struct FGBufferInfo
	{
		VkDeviceSize size;
		
		VkBufferUsageFlags usage;
		FGBufferHandle handle;
	};

	struct FGTextureInfo
	{
		VkFormat format;
		VkExtent2D extent{ 0, 0 };
		FGResizeMode resizeMode{ FGResizeMode::Fixed };
		uint32_t mipLevels{ 1 };

		VkImageUsageFlags usage{ 0 };
		FGTextureHandle handle;
	};

	struct FGReferenceInfo
	{
		FGResourceHandle handle;
	};

	using FGResourceInfo = std::variant<FGBufferInfo, FGTextureInfo, FGReferenceInfo>;

	struct FGResource
	{
		std::string name;
		FGResourceUsage usage;
		FGResourceInfo info;
	};





	struct VulkanAccessInfo
	{
		VkPipelineStageFlags stage;
		VkAccessFlags access;
		VkImageLayout layout;
	};

	static auto toAccessInfo(FGResourceUsage usage) -> VulkanAccessInfo
	{
		switch (usage)
		{
		default:
		case FGResourceUsage::None:
			return VulkanAccessInfo{
				.stage = 0,
				.access = 0,
				.layout = VK_IMAGE_LAYOUT_UNDEFINED
			};
		case FGResourceUsage::ColorAttachment:
			return VulkanAccessInfo{
				.stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
				.access = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
				.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
			};
		case FGResourceUsage::DepthStencilAttachment:
			return VulkanAccessInfo{
				.stage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
				.access = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
				.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
			};
		case FGResourceUsage::FragmentReadSampled:
			return VulkanAccessInfo{
				.stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
				.access = VK_ACCESS_SHADER_READ_BIT,
				.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
			};
		case FGResourceUsage::ComputeReadStorage:
			return VulkanAccessInfo{
				.stage = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
				.access = VK_ACCESS_SHADER_READ_BIT,
				.layout = VK_IMAGE_LAYOUT_GENERAL
			};
		case FGResourceUsage::ComputeWriteStorage:
			return VulkanAccessInfo{
				.stage = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
				.access = VK_ACCESS_SHADER_WRITE_BIT,
				.layout = VK_IMAGE_LAYOUT_GENERAL
			};
		case FGResourceUsage::ComputeReadSampled:
			return VulkanAccessInfo{
				.stage = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
				.access = VK_ACCESS_SHADER_READ_BIT,
				.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
			};
		case FGResourceUsage::TransferSrc:
			return VulkanAccessInfo{
				.stage = VK_PIPELINE_STAGE_TRANSFER_BIT,
				.access = VK_ACCESS_TRANSFER_READ_BIT,
				.layout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL
			};
		case FGResourceUsage::TransferDst:
			return VulkanAccessInfo{
				.stage = VK_PIPELINE_STAGE_TRANSFER_BIT,
				.access = VK_ACCESS_TRANSFER_WRITE_BIT,
				.layout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
			};
		case FGResourceUsage::Present:
			return VulkanAccessInfo{
				.stage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
				.access = 0,
				.layout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
			};
		}
	}

	static auto toImageUsage(FGResourceUsage usage) -> VkImageUsageFlags
	{
		switch (usage)
		{
		case FGResourceUsage::ColorAttachment:
			return VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		case FGResourceUsage::DepthStencilAttachment:
			return VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		case FGResourceUsage::FragmentReadSampled:
			return VK_IMAGE_USAGE_SAMPLED_BIT;
		case FGResourceUsage::ComputeReadStorage:
		case FGResourceUsage::ComputeWriteStorage:
			return VK_IMAGE_USAGE_STORAGE_BIT;
		case FGResourceUsage::ComputeReadSampled:
			return VK_IMAGE_USAGE_SAMPLED_BIT;
		case FGResourceUsage::TransferSrc:
			return VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
		case FGResourceUsage::TransferDst:
			return VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		case FGResourceUsage::Present:
			return VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT; // Typically also a color attachment
		default:
			return 0;
		}
	}

	static auto toBufferUsage(FGResourceUsage usage) -> VkBufferUsageFlags
	{
		switch (usage)
		{
		case FGResourceUsage::ComputeReadStorage:
		case FGResourceUsage::ComputeWriteStorage:
			return VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
		case FGResourceUsage::TransferSrc:
			return VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		case FGResourceUsage::TransferDst:
			return VK_BUFFER_USAGE_TRANSFER_DST_BIT;
		default:
			return 0;
		}
	}
}
