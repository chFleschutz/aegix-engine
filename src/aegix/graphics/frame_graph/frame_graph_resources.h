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
