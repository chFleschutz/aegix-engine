#include "render_system.h"

namespace Aegix::Graphics
{
	RenderSystem::RenderSystem(VulkanDevice& device, VkDescriptorSetLayout globalSetLayout)
		: m_device{ device } 
	{
	}
}
