#pragma once

#include "graphics/buffer.h"
#include "graphics/descriptors.h"
#include "graphics/material.h"
#include "graphics/pipeline.h"
#include "graphics/swap_chain.h"
#include "scene/entity.h"
#include "scene/scene.h"
#include "utils/math_utils.h"

namespace Aegix::Graphics
{
	class RenderSystem
	{
	public:
		RenderSystem(VulkanDevice& device) : m_device{ device } {}
		virtual ~RenderSystem()
		{
			// TODO: Wrap this in own class
			if (m_pipelineLayout)
				vkDestroyPipelineLayout(m_device.device(), m_pipelineLayout, nullptr);
		}

		virtual void initialize(Scene::Scene& scene, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout) = 0;
		virtual void render(VkCommandBuffer commandBuffer, Scene::Scene& scene) = 0;

	protected:
		VulkanDevice& m_device;

		std::unique_ptr<DescriptorPool> m_descriptorPool;
		std::unique_ptr<DescriptorSetLayout> m_descriptorSetLayout;
		VkPipelineLayout m_pipelineLayout = nullptr;
		std::unique_ptr<Pipeline> m_pipeline;
	};



	// Example Material

	struct ExampleMaterialData
	{
		glm::mat4 model;
	};

	class ExampleMaterial : public BaseMaterial
	{
	public:
		ExampleMaterial(VulkanDevice& device) : BaseMaterial(device) {}

		void initialize(DescriptorSetLayout& setLayout, DescriptorPool& pool)
		{
			m_uniformBuffers.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);
			m_descriptorSets.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);

			for (int i = 0; i < SwapChain::MAX_FRAMES_IN_FLIGHT; i++)
			{
				// Create uniform buffer
				m_uniformBuffers[i] = std::make_unique<Buffer>(m_device, sizeof(ExampleMaterialData), 1, 
					VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
				m_uniformBuffers[i]->map();

				// Allocate and write descriptor set
				auto bufferInfo = m_uniformBuffers[i]->descriptorInfo();
				DescriptorWriter(setLayout, pool)
					.writeBuffer(0, &bufferInfo)
					.build(m_descriptorSets[i]);
			}
		}

	private:
		std::vector<std::unique_ptr<Buffer>> m_uniformBuffers;
	};

	class ExampleRenderSystem : public RenderSystem
	{
	public:
		ExampleRenderSystem(VulkanDevice& device) : RenderSystem(device) {}

		virtual void initialize(Scene::Scene& scene, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout) override
		{
			auto view = scene.viewEntitiesByType<Component::Material>();
			
			createDescriptorPool(view.size());
			createDescriptorSetLayouts();
			createPipelineLayout(globalSetLayout);
			createPipeline(renderPass);

			// Create materials
			for (auto&& [entity, material] : view.each())
			{
				assert(material.material);

				material.material->initialize(*m_descriptorSetLayout, *m_descriptorPool);
			}

		}

		virtual void render(VkCommandBuffer commandBuffer, Scene::Scene& scene) override
		{
			m_pipeline->bind(commandBuffer);

			// Iterate over entities with material
			for (auto&& [entity, transform, mesh, material] : scene.viewEntitiesByType<Component::Transform, Component::Mesh, Component::Material>().each())
			{
				auto descriptorSet = material.material->descriptorSet(0);
				vkCmdBindDescriptorSets(commandBuffer,
					VK_PIPELINE_BIND_POINT_GRAPHICS,
					m_pipelineLayout,
					0, 1,
					&descriptorSet,
					0, nullptr
				);

				// TODO: remove later
				struct SimplePushConstantData
				{	// max 128 bytes
					Matrix4 modelMatrix{ 1.0f };
					Matrix4 normalMatrix{ 1.0f };
				};

				// TODO: transfer color as uniform
				Matrix4 colorNormalMatrix = MathLib::normalMatrix(transform.rotation, transform.scale);
				colorNormalMatrix[3] = mesh.color.rgba();

				SimplePushConstantData push{};
				push.modelMatrix = MathLib::tranformationMatrix(transform.location, transform.rotation, transform.scale);
				push.normalMatrix = colorNormalMatrix;

				vkCmdPushConstants(
					commandBuffer,
					m_pipelineLayout,
					VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
					0,
					sizeof(SimplePushConstantData),
					&push);

				mesh.model->bind(commandBuffer);
				mesh.model->draw(commandBuffer);
			}
		}

	private:
		void createDescriptorPool(uint32_t maxSets)
		{
			// TODO: use single pool, see: https://vkguide.dev/docs/extra-chapter/abstracting_descriptors/
			m_descriptorPool = DescriptorPool::Builder(m_device)
				.setMaxSets(maxSets)
				.addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, maxSets)
				.build();
		}

		void createDescriptorSetLayouts()
		{
			m_descriptorSetLayout = DescriptorSetLayout::Builder(m_device)
				.addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
				.build();
		}

		void createPipelineLayout(VkDescriptorSetLayout globalSetLayout)
		{
			std::vector<VkDescriptorSetLayout> descriptorSetLayouts{ globalSetLayout, m_descriptorSetLayout->descriptorSetLayout()};

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
