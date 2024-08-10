#pragma once

#include "scene/scene.h"
#include "graphics/descriptors.h"
#include "graphics/pipeline.h"
#include "graphics/buffer.h"
#include "utils/math_utils.h"

namespace Aegix::Graphics
{
	class RenderSystem
	{
	public:
		RenderSystem(VulkanDevice& device) : m_device{ device } {}
		virtual ~RenderSystem() = default;

		virtual void initialize(VkRenderPass renderPass) = 0;
		virtual void render() = 0;

	protected:
		VulkanDevice& m_device;

		std::vector<std::unique_ptr<DescriptorSetLayout>> m_descriptorSetLayouts;
		VkPipelineLayout m_pipelineLayout = nullptr;
		std::unique_ptr<Pipeline> m_pipeline;
	};



	struct ExampleMaterial
	{

	};

	class ExampleRenderSystem : public RenderSystem
	{
	public:
		ExampleRenderSystem(VulkanDevice& device) : RenderSystem(device) {}

		virtual void initialize(VkRenderPass renderPass) override
		{
			createDescriptorSetLayouts();
			createPipelineLayout();
			createPipeline(renderPass);

			// Create Descriptor Sets and Write
		}

		virtual void render() override
		{
			// Bind Pipeline

			// Bind global descriptor set

			// Iterate over entities with material

				// Bind material descriptor sets
				// Bind vertex/index buffer
				// Draw
		}

	private:
		void createDescriptorSetLayouts()
		{
			auto globalSetLayout = DescriptorSetLayout::Builder(m_device)
				.addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
				.build();

			auto materialSetLayout = DescriptorSetLayout::Builder(m_device)
				.addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
				.build();

			m_descriptorSetLayouts.push_back(std::move(globalSetLayout));
			m_descriptorSetLayouts.push_back(std::move(materialSetLayout));
		}

		void createPipelineLayout()
		{
			std::vector<VkDescriptorSetLayout> descriptorSetLayouts{};
			for (auto& layout : m_descriptorSetLayouts)
			{
				descriptorSetLayouts.push_back(layout->descriptorSetLayout());
			}

			VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
			pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
			pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
			pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();

			// TODO: Add push constants
			pipelineLayoutInfo.pushConstantRangeCount = 0;
			pipelineLayoutInfo.pPushConstantRanges = nullptr;

			if (vkCreatePipelineLayout(m_device.device(), &pipelineLayoutInfo, nullptr, &m_pipelineLayout) != VK_SUCCESS)
				throw std::runtime_error("failed to create pipeline layout");
		}

		void createPipeline(VkRenderPass renderPass)
		{
			PipelineConfigInfo pipelineConfig{};
			Pipeline::defaultPipelineConfigInfo(pipelineConfig);
			pipelineConfig.renderPass = renderPass;
			pipelineConfig.pipelineLayout = m_pipelineLayout;

			std::string vertShaderPath = SHADER_DIR "simple_shader.vert.spv";
			std::string fragShaderPath = SHADER_DIR "simple_shader.frag.spv";

			m_pipeline = std::make_unique<Pipeline>(m_device, vertShaderPath, fragShaderPath, pipelineConfig);
		}
	};
}
