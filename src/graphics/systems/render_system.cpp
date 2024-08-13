#include "render_system.h"

namespace Aegix::Graphics
{
	Graphics::RenderSystem::RenderSystem(VulkanDevice& device)
		: m_device{ device } 
	{
	}
}
