#pragma once

#include "graphics/vulkan/volk_include.h"

#include <vk_mem_alloc.h>

namespace Aegix::Graphics
{
	struct FGHandle
	{
		static constexpr uint32_t INVALID_HANDLE = std::numeric_limits<uint32_t>::max();

		uint32_t handle{ INVALID_HANDLE };

		[[nodiscard]] auto operator==(const FGHandle& other) const -> bool { return handle == other.handle; }
		[[nodiscard]] auto isValid() const -> bool { return handle != INVALID_HANDLE; }
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
		uint32_t instanceCount{ 1 };
		VkBufferUsageFlags usage{ 0 };
		VmaAllocationCreateFlags allocFlags{ 0 };
		
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

	struct FGResource
	{
		using Info = std::variant<FGBufferInfo, FGTextureInfo, FGReferenceInfo>;

		enum class Usage
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
			IndirectBuffer,
		};

		struct AccessInfo
		{
			VkPipelineStageFlags stage;
			VkAccessFlags access;
			VkImageLayout layout;
		};

		std::string name;
		Usage usage;
		Info info;

		static auto toAccessInfo(Usage usage) -> AccessInfo;
		static auto toImageUsage(Usage usage) -> VkImageUsageFlags;
		static auto toBufferUsage(Usage usage) -> VkBufferUsageFlags;
	};
}
