#pragma once

#include "graphics/device.h"
#include "graphics/descriptors.h"
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
		[[nodiscard]] static auto descriptorPool() -> DescriptorPool& { return *instance().m_descriptorPool; }
		[[nodiscard]] static auto deletionQueue() -> DeletionQueue& { return instance().m_deletionQueue; }

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
		std::unique_ptr<DescriptorPool> m_descriptorPool{};
		DeletionQueue m_deletionQueue{};
	};
}