#include "pbs_render_system.h"

namespace Aegix::Graphics
{
	PBSMaterialInstance::PBSMaterialInstance(VulkanDevice& device, DescriptorSetLayout& setLayout, DescriptorPool& pool, PBSMaterial::Data data)
		: m_uniformBuffer{ device, data }
	{
		m_descriptorSet = DescriptorSet::Builder(device, pool, setLayout)
			.addBuffer(0, m_uniformBuffer)
			.build();
	}

	PBSRenderSystem::PBSRenderSystem(VulkanDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout)
		: RenderSystem(device, renderPass, globalSetLayout)
	{
		m_descriptorSetLayout = DescriptorSetLayout::Builder(m_device)
			.addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
			.build();

		m_pipelineLayout = PipelineLayout::Builder(m_device)
			.addDescriptorSetLayout(globalSetLayout)
			.addDescriptorSetLayout(m_descriptorSetLayout->descriptorSetLayout())
			.addPushConstantRange(VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(PushConstantData))
			.build();

		m_pipeline = Pipeline::Builder(m_device)
			.setRenderPass(renderPass)
			.setPipelineLayout(m_pipelineLayout->pipelineLayout())
			.addShaderStage(VK_SHADER_STAGE_VERTEX_BIT, SHADER_DIR "pbs.vert.spv")
			.addShaderStage(VK_SHADER_STAGE_FRAGMENT_BIT, SHADER_DIR "pbs.frag.spv")
			.build();
	}

	void PBSRenderSystem::render(const FrameInfo& frameInfo)
	{
		m_pipeline->bind(frameInfo.commandBuffer);

		// Global Descriptor Set
		vkCmdBindDescriptorSets(
			frameInfo.commandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			m_pipelineLayout->pipelineLayout(),
			0, 1,
			&frameInfo.globalDescriptorSet,
			0, nullptr
		);

		PBSMaterialInstance* lastMaterial = nullptr;
		auto view = frameInfo.scene->viewEntities<Component::Transform, Component::Mesh, PBSMaterial>();
		for (auto&& [entity, transform, mesh, material] : view.each())
		{
			if (mesh.staticMesh == nullptr || material.instance == nullptr)
				continue;

			// Material Descriptor Set
			if (lastMaterial != material.instance.get())
			{
				lastMaterial = material.instance.get();
				auto& descriptorSet = material.instance->m_descriptorSet;
				assert(descriptorSet != nullptr && "Material descriptor set is null");

				vkCmdBindDescriptorSets(
					frameInfo.commandBuffer,
					VK_PIPELINE_BIND_POINT_GRAPHICS,
					m_pipelineLayout->pipelineLayout(),
					1, 1,
					&descriptorSet->descriptorSet(frameInfo.frameIndex),
					0, nullptr
				);
			}

			// Push Constants
			PushConstantData push{};
			push.modelMatrix = MathLib::tranformationMatrix(transform.location, transform.rotation, transform.scale);
			push.normalMatrix = MathLib::normalMatrix(transform.rotation, transform.scale);
			vkCmdPushConstants(frameInfo.commandBuffer,
				m_pipelineLayout->pipelineLayout(),
				VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
				0,
				sizeof(push),
				&push
			);

			// Draw
			mesh.staticMesh->bind(frameInfo.commandBuffer);
			mesh.staticMesh->draw(frameInfo.commandBuffer);
		}
	}
}