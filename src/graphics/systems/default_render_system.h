#pragma once

#include "graphics/systems/render_system.h"
#include "graphics/uniform_buffer.h"

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
			glm::vec4 color;
		};

		std::shared_ptr<DefaultMaterialInstance> instance;
	};

	/// @brief Instance of a DefaultMaterial that holds the uniform buffer with the material data
	class DefaultMaterialInstance
	{
	public:
		DefaultMaterialInstance(VulkanDevice& device, DescriptorSetLayout& setLayout, DescriptorPool& pool);

		void setData(const DefaultMaterial::Data& data) { m_uniformBuffer.setData(data); }

	private:
		UniformBuffer<DefaultMaterial::Data> m_uniformBuffer;
		std::array<VkDescriptorSet, SwapChain::MAX_FRAMES_IN_FLIGHT> m_descriptorSets;

		friend DefaultRenderSystem;
	};

	/// @brief Render system for rendering entities with a DefaultMaterial component
	class DefaultRenderSystem : public RenderSystem
	{
	public:
		struct PushConstantData // max 128 bytes
		{
			Matrix4 modelMatrix{ 1.0f };
			Matrix4 normalMatrix{ 1.0f };
		};

		DefaultRenderSystem(VulkanDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);

		virtual void render(const FrameInfo& frameInfo) override;
	};
}
