#pragma once

#include "graphics/systems/render_system.h"
#include "graphics/uniform_buffer.h"
#include "graphics/texture.h"

namespace Aegix::Graphics
{
	class DefaultRenderSystem;
	class DefaultMaterialInstance;

	/// @brief Component for the default material that holds the material instance
	/// @note This struct also has to define the RenderSystem and Instance types
	struct DefaultMaterial
	{
		using RenderSystem = DefaultRenderSystem;
		using Instance = DefaultMaterialInstance;

		struct Data
		{
			float shininess = 32.0f;
		};

		std::shared_ptr<DefaultMaterialInstance> instance;
	};

	/// @brief Instance of a DefaultMaterial that holds the uniform buffer with the material data
	class DefaultMaterialInstance
	{
	public:
		DefaultMaterialInstance(VulkanDevice& device, DescriptorSetLayout& setLayout, DescriptorPool& pool,
			std::shared_ptr<Texture> texture);

		void setData(const DefaultMaterial::Data& data);
		const DefaultMaterial::Data& data() const { return m_uniformBuffer.data(); }

	private:
		UniformBuffer<DefaultMaterial::Data> m_uniformBuffer;
		std::shared_ptr<Texture> m_texture;
		std::array<VkDescriptorSet, SwapChain::MAX_FRAMES_IN_FLIGHT> m_descriptorSets;

		friend DefaultRenderSystem;
	};

	/// @brief Render system for rendering entities with a DefaultMaterial component
	class DefaultRenderSystem : public RenderSystem
	{
	public:
		struct PushConstantData // max 128 bytes
		{
			glm::mat4 modelMatrix{ 1.0f };
			glm::mat4 normalMatrix{ 1.0f };
		};

		DefaultRenderSystem(VulkanDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);

		virtual void render(const FrameInfo& frameInfo) override;
	};
}
