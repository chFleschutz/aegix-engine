#include "render_system.h"

namespace Aegix::Graphics
{
	Graphics::RenderSystem::RenderSystem(VulkanDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout)
		: m_device{ device } 
	{
	}
}
