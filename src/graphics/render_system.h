#pragma once

#include "scene/scene.h"
#include "graphics/descriptors.h"
#include "graphics/pipeline.h"

namespace VEGraphics
{
	struct BaseMaterial
	{
	public:
		BaseMaterial() = default;
		virtual ~BaseMaterial() = default;

		virtual void initialize()
		{
			// TODO: Add RenderSystem to Renderer
		}

	};

	class RenderSystem
	{
	public:
		RenderSystem() = default;
		virtual ~RenderSystem() = default;

		virtual void initialize() = 0;

		virtual void render() = 0;

	protected:
		void addDescriptorSetLayout(std::unique_ptr<DescriptorSetLayout> layout)
		{
			mDescriptorSets.push_back(std::move(layout));
		}

	private:
		void createPipelineLayout();
		void createPipeline(VkRenderPass renderPass);

		VulkanDevice& m_device;

		std::vector<std::unique_ptr<DescriptorSetLayout>> m_DescriptorSetLayouts;

		VkPipelineLayout mPipelineLayout;
		std::unique_ptr<Pipeline> m_Pipeline;
	};



	struct DefaultMaterial : public BaseMaterial
	{

	};

	class DefaultRenderSystem : public RenderSystem
	{
	public:
		DefaultRenderSystem(VulkanDevice& device) : m_device(device) {}
		virtual ~DefaultRenderSystem() = default;

		virtual void initialize() override
		{
			auto globalSetLayout = DescriptorSetLayout::Builder(m_device)
				.addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
				.build();

			auto materialSetLayout = DescriptorSetLayout::Builder(m_device)
				.addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT)
				.build();

			addDescriptorSetLayout(std::move(globalSetLayout));
			addDescriptorSetLayout(std::move(materialSetLayout));
		}

		virtual void render(VEScene::Scene& scene) override
		{
			auto& view = scene.viewEntitiesByType<DefaultMaterial>();
			for (auto&& [entity, material] : view)
			{
				// TODO: render entity with material
			}
		}

	private:
		VulkanDevice& m_device;
	};

} // namespace VEGraphics
