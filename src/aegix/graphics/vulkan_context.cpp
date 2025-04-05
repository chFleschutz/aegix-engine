#include "pch.h"

#include "vulkan_context.h"

namespace Aegix::Graphics
{
	auto VulkanContext::instance() -> VulkanContext&
	{
		static VulkanContext instance;
		return instance;
	}

	auto VulkanContext::initialize(Core::Window& window) -> VulkanContext&
	{
		auto& context = instance();
		context.m_device.initialize(window);
		return context;
	}
}
