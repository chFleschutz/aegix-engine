#pragma once

#include "graphics/systems/render_system.h"

namespace Aegix::Graphics
{
	class PBSRenderSystem;
	class PBSMaterialInstance;

	struct PBSMaterial
	{
		using RenderSystem = PBSRenderSystem;
		using Instance = PBSMaterialInstance;

		std::shared_ptr<PBSMaterialInstance> instance;
	};

	class PBSMaterialInstance
	{
	public:

	};

	class PBSRenderSystem : public RenderSystem
	{
	public:
		struct PushConstantData // max 128 bytes
		{
			glm::mat4 modelMatrix{ 1.0f };
			glm::mat4 normalMatrix{ 1.0f };
		};

		PBSRenderSystem(VulkanDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);

		virtual void render(const FrameInfo& frameInfo) override;
	};
}
