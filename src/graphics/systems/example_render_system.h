#pragma once

#include "graphics/render_system.h"
#include "graphics/uniform_buffer.h"

namespace Aegix::Graphics
{
	class ExampleMaterial;
	class ExampleRenderSystem;



	// Example Material

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



	// Example Render System

	class ExampleRenderSystem : public RenderSystem
	{
	public:
		struct PushConstantData // max 128 bytes
		{	
			Matrix4 modelMatrix{ 1.0f };
			Matrix4 normalMatrix{ 1.0f };
		};

		ExampleRenderSystem(VulkanDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout)
			: RenderSystem(device)
		{
			m_descriptorSetLayout = DescriptorSetLayout::Builder(m_device)
				.addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
				.build();

			std::vector<VkDescriptorSetLayout> descriptorSetLayouts{
				globalSetLayout,
				m_descriptorSetLayout->descriptorSetLayout(),
			};
			createPipelineLayout(descriptorSetLayouts, sizeof(PushConstantData));

			std::string vertShaderPath = SHADER_DIR "example.vert.spv";
			std::string fragShaderPath = SHADER_DIR "example.frag.spv";
			createPipeline(renderPass, vertShaderPath, fragShaderPath);
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

			auto view = frameInfo.scene->viewEntitiesByType<Component::Transform, Component::Mesh, ExampleMaterial>();
			for (auto&& [entity, transform, mesh, material] : view.each())
			{
				// Descriptor Set
				vkCmdBindDescriptorSets(frameInfo.commandBuffer,
					VK_PIPELINE_BIND_POINT_GRAPHICS,
					m_pipelineLayout,
					1, 1,
					&material.material->m_descriptorSets[frameInfo.frameIndex],
					0, nullptr
				);

				// Push Constants
				PushConstantData push{};
				push.modelMatrix = MathLib::tranformationMatrix(transform.location, transform.rotation, transform.scale);
				push.normalMatrix = MathLib::normalMatrix(transform.rotation, transform.scale);

				vkCmdPushConstants(frameInfo.commandBuffer,
					m_pipelineLayout,
					VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
					0,
					sizeof(push),
					&push
				);

				// Draw
				mesh.model->bind(frameInfo.commandBuffer);
				mesh.model->draw(frameInfo.commandBuffer);
			}
		}
	};
}
