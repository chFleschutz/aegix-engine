#include "default_render_system.h"

#include "scene/components.h"

namespace Aegix::Graphics
{
	DefaultMaterialInstance::DefaultMaterialInstance(VulkanDevice& device, DescriptorSetLayout& setLayout, DescriptorPool& pool)
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

	DefaultRenderSystem::DefaultRenderSystem(VulkanDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout)
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

	void DefaultRenderSystem::render(const FrameInfo& frameInfo)
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

		auto view = frameInfo.scene->viewEntitiesByType<Component::Transform, Component::Mesh, DefaultMaterial>();
		for (auto&& [entity, transform, mesh, material] : view.each())
		{
			// Descriptor Set
			vkCmdBindDescriptorSets(frameInfo.commandBuffer,
				VK_PIPELINE_BIND_POINT_GRAPHICS,
				m_pipelineLayout,
				1, 1,
				&material.instance->m_descriptorSets[frameInfo.frameIndex],
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
}
