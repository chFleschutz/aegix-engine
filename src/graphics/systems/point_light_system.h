#pragma once

#include "graphics/systems/render_system.h"

namespace Aegix::Graphics
{
	/// @brief Render system for rendering point lights
	/// @note This system only renders a representation of the lights, lighting can be done without this system
	class PointLightSystem : public RenderSystem
	{
	public:
		struct PointLightPushConstants // max 128 bytes
		{
			glm::vec4 position{};
			glm::vec4 color{};
			float radius;
		};

		PointLightSystem(VulkanDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);

		virtual void render(const FrameInfo& frameInfo) override;
	};
} 
