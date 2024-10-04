#pragma once

#include "graphics/frame_info.h"
#include "graphics/systems/render_system_collection.h"

namespace Aegix::Graphics
{
	class RenderPass
	{
	public:
		virtual ~RenderPass() = default;

		virtual void render(FrameInfo& frameInfo) = 0;

		template<typename T>
		RenderSystem& addRenderSystem(VulkanDevice& device, VkRenderPass renderpass)
		{
			assert(m_globalSetLayout && "Global descriptor set layout not initialized");

			return m_renderSystemCollection.addRenderSystem<T>(device, renderpass, m_globalSetLayout->descriptorSetLayout());
		}

	protected:
		RenderSystemCollection m_renderSystemCollection;

		std::unique_ptr<DescriptorSetLayout> m_globalSetLayout;
	};
}