#pragma once

#include "graphics/material.h"
#include "graphics/render_system.h"
#include "graphics/uniform_buffer.h"

namespace Aegix::Graphics
{
	class ExampleMaterial;
	class ExampleRenderSystem;

	template<>
	struct RenderSystemRef<ExampleMaterial>
	{
		using type = ExampleRenderSystem;
	};


	struct ExampleMaterial
	{
		struct Data
		{
			glm::vec4 color;
		};

		class Instance
		{
		public:
			Instance(VulkanDevice& device, DescriptorSetLayout& setLayout, DescriptorPool& pool)
				: m_uniformBuffer(device, setLayout, pool) 
			{
				for (int i = 0; i < SwapChain::MAX_FRAMES_IN_FLIGHT; i++)
				{
					auto bufferInfo = m_uniformBuffer.descriptorInfo(i);
					DescriptorWriter(setLayout, pool)
						.writeBuffer(0, &bufferInfo)
						.build(m_descriptorSets[i]);
				}
			}

			void setData(const ExampleMaterial::Data& data) { m_uniformBuffer.setData(data); }

		private:
			UniformBuffer<ExampleMaterial::Data> m_uniformBuffer;
			std::array<VkDescriptorSet, SwapChain::MAX_FRAMES_IN_FLIGHT> m_descriptorSets;

			friend ExampleRenderSystem;
		};

		std::shared_ptr<ExampleMaterial::Instance> material;
	};



	class ExampleRenderSystem : public RenderSystem
	{
	public:
		// TODO: remove later
		struct SimplePushConstantData
		{	// max 128 bytes
			Matrix4 modelMatrix{ 1.0f };
			Matrix4 normalMatrix{ 1.0f };
		};

		ExampleRenderSystem(VulkanDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout)
			: RenderSystem(device)
		{
			createDescriptorSetLayouts();
			createPipelineLayout(globalSetLayout);
			createPipeline(renderPass);
		}

		virtual void render(const FrameInfo& frameInfo) override
		{
			m_pipeline->bind(frameInfo.commandBuffer);

			vkCmdBindDescriptorSets(
				frameInfo.commandBuffer,
				VK_PIPELINE_BIND_POINT_GRAPHICS,
				m_pipelineLayout,
				0, 1,
				&frameInfo.globalDescriptorSet,
				0, nullptr
			);

			// Iterate over entities with material
			auto view = frameInfo.scene->viewEntitiesByType<Component::Transform, Component::Mesh, ExampleMaterial>();
			for (auto&& [entity, transform, mesh, material] : view.each())
			{
				auto descriptorSet = material.material->m_descriptorSets[frameInfo.frameIndex];
				vkCmdBindDescriptorSets(frameInfo.commandBuffer,
					VK_PIPELINE_BIND_POINT_GRAPHICS,
					m_pipelineLayout,
					1, 1,
					&descriptorSet,
					0, nullptr
				);

				// TODO: transfer color as uniform
				Matrix4 colorNormalMatrix = MathLib::normalMatrix(transform.rotation, transform.scale);
				colorNormalMatrix[3] = mesh.color.rgba();

				SimplePushConstantData push{};
				push.modelMatrix = MathLib::tranformationMatrix(transform.location, transform.rotation, transform.scale);
				push.normalMatrix = colorNormalMatrix;

				vkCmdPushConstants(frameInfo.commandBuffer,
					m_pipelineLayout,
					VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
					0,
					sizeof(SimplePushConstantData),
					&push
				);

				mesh.model->bind(frameInfo.commandBuffer);
				mesh.model->draw(frameInfo.commandBuffer);
			}
		}

	private:
		void createDescriptorSetLayouts()
		{
			m_descriptorSetLayout = DescriptorSetLayout::Builder(m_device)
				.addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
				.build();
		}

		void createPipelineLayout(VkDescriptorSetLayout globalSetLayout)
		{
			std::vector<VkDescriptorSetLayout> descriptorSetLayouts {
				globalSetLayout, 
				m_descriptorSetLayout->descriptorSetLayout(), 
			};

			VkPushConstantRange pushConstantRange{};
			pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
			pushConstantRange.offset = 0;
			pushConstantRange.size = sizeof(SimplePushConstantData);

			VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
			pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
			pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
			pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
			pipelineLayoutInfo.pushConstantRangeCount = 1;
			pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

			if (vkCreatePipelineLayout(m_device.device(), &pipelineLayoutInfo, nullptr, &m_pipelineLayout) != VK_SUCCESS)
				throw std::runtime_error("failed to create pipeline layout");
		}

		void createPipeline(VkRenderPass renderPass)
		{
			PipelineConfigInfo pipelineConfig{};
			Pipeline::defaultPipelineConfigInfo(pipelineConfig);
			pipelineConfig.renderPass = renderPass;
			pipelineConfig.pipelineLayout = m_pipelineLayout;

			std::string vertShaderPath = SHADER_DIR "example.vert.spv";
			std::string fragShaderPath = SHADER_DIR "example.frag.spv";

			m_pipeline = std::make_unique<Pipeline>(m_device, vertShaderPath, fragShaderPath, pipelineConfig);
		}
	};
}
