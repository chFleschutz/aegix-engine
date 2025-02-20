#include "pbs_render_system.h"

namespace Aegix::Graphics
{
	PBSMaterialInstance::PBSMaterialInstance(VulkanDevice& device, DescriptorSetLayout& setLayout, DescriptorPool& pool,
		std::shared_ptr<Texture> albedo, std::shared_ptr<Texture> normal, std::shared_ptr<Texture> metalRoughness,
		std::shared_ptr<Texture> ao, std::shared_ptr<Texture> emissive, PBSMaterial::Data data)
		: m_albedoTexture{ albedo }, m_normalTexture{ normal },	m_metalRoughnessTexture{ metalRoughness },
		m_aoTexture{ ao }, m_emissiveTexture{ emissive }, m_uniformBuffer{ device, data }
	{
		m_descriptorSet = DescriptorSet::Builder(device, pool, setLayout)
			.addBuffer(0, m_uniformBuffer)
			.addTexture(1, m_albedoTexture)
			.addTexture(2, m_normalTexture)
			.addTexture(3, m_metalRoughnessTexture)
			.addTexture(4, m_aoTexture)
			.addTexture(5, m_emissiveTexture)
			.build();
	}

	PBSRenderSystem::PBSRenderSystem(VulkanDevice& device, VkDescriptorSetLayout globalSetLayout)
		: RenderSystem(device, globalSetLayout)
	{
		m_descriptorSetLayout = DescriptorSetLayout::Builder(m_device)
			.addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
			.addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
			.addBinding(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
			.addBinding(3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
			.addBinding(4, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
			.addBinding(5, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
			.build();

		m_pipelineLayout = PipelineLayout::Builder(m_device)
			.addDescriptorSetLayout(globalSetLayout)
			.addDescriptorSetLayout(*m_descriptorSetLayout)
			.addPushConstantRange(VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(PushConstantData))
			.build();

		m_pipeline = Pipeline::Builder(m_device, *m_pipelineLayout)
			.addShaderStage(VK_SHADER_STAGE_VERTEX_BIT, SHADER_DIR "gbuffer.vert.spv")
			.addShaderStage(VK_SHADER_STAGE_FRAGMENT_BIT, SHADER_DIR "gbuffer.frag.spv")
			.addColorAttachment(VK_FORMAT_R16G16B16A16_SFLOAT)
			.addColorAttachment(VK_FORMAT_R16G16B16A16_SFLOAT)
			.addColorAttachment(VK_FORMAT_R8G8B8A8_UNORM)
			.addColorAttachment(VK_FORMAT_R8G8B8A8_UNORM)
			.addColorAttachment(VK_FORMAT_R8G8B8A8_UNORM)
			.setDepthAttachment(VK_FORMAT_D32_SFLOAT)
			.build();
	}

	void PBSRenderSystem::render(const FrameInfo& frameInfo, VkDescriptorSet globalSet)
	{
		m_pipeline->bind(frameInfo.commandBuffer);

		// Global Descriptor Set
		vkCmdBindDescriptorSets(
			frameInfo.commandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			m_pipelineLayout->pipelineLayout(),
			0, 1,
			&globalSet,
			0, nullptr
		);

		PBSMaterialInstance* lastMaterial = nullptr;
		auto view = frameInfo.scene.viewEntities<Component::Transform, Component::Mesh, PBSMaterial>();
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