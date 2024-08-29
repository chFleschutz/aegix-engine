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
			alignas(16) glm::vec3 albedo{ 1.0f, 1.0f, 1.0f };
			alignas(16) glm::vec3 emissive{ 1.0f, 1.0f, 1.0f };
			alignas(4) float metallic = 1.0f;
			alignas(4) float roughness = 1.0f;
			alignas(4) float ambientOcclusion = 1.0f;
		};

		std::shared_ptr<PBSMaterialInstance> instance;
	};

	class PBSMaterialInstance
	{
	public:
		PBSMaterialInstance(VulkanDevice& device, DescriptorSetLayout& setLayout, DescriptorPool& pool, 
			std::shared_ptr<Texture> albedo, std::shared_ptr<Texture> normal, std::shared_ptr<Texture> metalRoughness,
			std::shared_ptr<Texture> ao, std::shared_ptr<Texture> emissive, PBSMaterial::Data data = {});

	private:
		UniformBuffer<PBSMaterial::Data> m_uniformBuffer;
		std::shared_ptr<Texture> m_albedoTexture;
		std::shared_ptr<Texture> m_normalTexture;
		std::shared_ptr<Texture> m_metalRoughnessTexture;
		std::shared_ptr<Texture> m_aoTexture;
		std::shared_ptr<Texture> m_emissiveTexture;
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
