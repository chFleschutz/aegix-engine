#pragma once

#include "graphics/device.h"
#include "graphics/deletion_queue.h"

namespace Aegix::Graphics
{
	class VulkanContext
	{
	public:
		~VulkanContext() = default;
		VulkanContext(const VulkanContext&) = delete;
		VulkanContext(VulkanContext&&) = delete;

		auto operator=(const VulkanContext&) -> VulkanContext& = delete;
		auto operator=(VulkanContext&&) -> VulkanContext& = delete;

		[[nodiscard]] static auto instance() -> VulkanContext&;
		[[nodiscard]] static auto device() -> VulkanDevice& { return instance().m_device; }

		static auto initialize(Core::Window& window) -> VulkanContext&;
		static void destroy();

		static void destroy(VkBuffer buffer, VmaAllocation allocation);
		static void destroy(VkImage image, VmaAllocation allocation);
		static void destroy(VkImageView view);
		static void destroy(VkSampler sampler);
		static void destroy(VkPipeline pipeline);
		static void destroy(VkPipelineLayout pipelineLayout);
		static void flushDeletionQueue(uint32_t frameIndex);

	private:
		VulkanContext() = default;

		// Device
		// Surface
		// Command Pool
		// Vma Allocator
		// Deletion Queue

		VulkanDevice m_device{};

		DeletionQueue m_deletionQueue{};
	};
}