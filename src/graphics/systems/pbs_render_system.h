#pragma once

#include "graphics/systems/render_system.h"
#include "graphics/uniform_buffer.h"

namespace Aegix::Graphics
{
	class PBSRenderSystem;
	class PBSMaterialInstance;

	struct PBSMaterial
	{
		using RenderSystem = PBSRenderSystem;
		using Instance = PBSMaterialInstance;

		struct Data
		{
			glm::vec3 albedo{ 1.0f, 1.0f, 1.0f };
			float metallic = 0.0f;
			float roughness = 1.0f;
			float ambientOcclusion = 0.0f;
		};

		std::shared_ptr<PBSMaterialInstance> instance;
	};

	class PBSMaterialInstance
	{
	public:
		PBSMaterialInstance(VulkanDevice& device, DescriptorSetLayout& setLayout, DescriptorPool& pool,
			PBSMaterial::Data data = {});

	private:
		UniformBuffer<PBSMaterial::Data> m_uniformBuffer;
		std::unique_ptr<DescriptorSet> m_descriptorSet;

		friend PBSRenderSystem;
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
