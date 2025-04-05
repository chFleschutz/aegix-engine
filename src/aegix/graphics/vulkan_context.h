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

		static auto initialize(Core::Window& window) -> VulkanContext&;
		[[nodiscard]] static auto instance() -> VulkanContext&;

		[[nodiscard]] auto device() -> VulkanDevice& { return m_device; }

	private:
		VulkanContext() = default;

		// Device
		// Surface
		// Command Pool
		// Vma Allocator
		// Deletion Queue

		VulkanDevice m_device{};
	};
}