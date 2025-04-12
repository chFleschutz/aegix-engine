#include "pch.h"

#include "vulkan_context.h"

namespace Aegix::Graphics
{
	auto VulkanContext::initialize(Core::Window& window) -> VulkanContext&
	{
		auto& context = instance();
		context.m_device.initialize(window);
		return context;
	}

	void VulkanContext::destroy()
	{
		auto& context = instance();
		context.m_deletionQueue.flushAll();
	}

	auto VulkanContext::instance() -> VulkanContext&
	{
		static VulkanContext instance;
		return instance;
	}

	void VulkanContext::destroy(VkBuffer buffer, VmaAllocation allocation)
	{
		if (buffer)
		{
			AGX_ASSERT_X(allocation, "Buffer and allocation must be valid");
			VulkanContext::instance().m_deletionQueue.schedule([=]() 
				{ 
					vmaDestroyBuffer(VulkanContext::instance().m_device.allocator(), buffer, allocation);
				});
		}
	}

	void VulkanContext::destroy(VkImage image, VmaAllocation allocation)
	{
		if (image)
		{
			AGX_ASSERT_X(allocation, "Image and allocation must be valid");
			VulkanContext::instance().m_deletionQueue.schedule([=]()
				{
					vmaDestroyImage(VulkanContext::instance().m_device.allocator(), image, allocation);
				});
		}
	}

	void VulkanContext::destroy(VkImageView view)
	{
		if (view)
		{
			VulkanContext::instance().m_deletionQueue.schedule([=]()
				{
					vkDestroyImageView(VulkanContext::instance().m_device.device(), view, nullptr);
				});
		}
	}

	void VulkanContext::destroy(VkSampler sampler)
	{
		if (sampler)
		{
			VulkanContext::instance().m_deletionQueue.schedule([=]()
				{
					vkDestroySampler(VulkanContext::instance().m_device.device(), sampler, nullptr);
				});
		}
	}

	void VulkanContext::destroy(VkPipeline pipeline)
	{
		if (pipeline)
		{
			VulkanContext::instance().m_deletionQueue.schedule([=]()
				{
					vkDestroyPipeline(VulkanContext::instance().m_device.device(), pipeline, nullptr);
				});
		}
	}

	void VulkanContext::destroy(VkPipelineLayout pipelineLayout)
	{
		if (pipelineLayout)
		{
			VulkanContext::instance().m_deletionQueue.schedule([=]()
				{
					vkDestroyPipelineLayout(VulkanContext::instance().m_device.device(), pipelineLayout, nullptr);
				});
		}
	}

	void VulkanContext::flushDeletionQueue(uint32_t frameIndex)
	{
		VulkanContext::instance().m_deletionQueue.flush(frameIndex);
	}
}
