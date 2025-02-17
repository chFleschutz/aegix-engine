#include "default_render_system.h"

#include "scene/components.h"

namespace Aegix::Graphics
{
	DefaultMaterialInstance::DefaultMaterialInstance(VulkanDevice& device, DescriptorSetLayout& setLayout, DescriptorPool& pool,
		std::shared_ptr<Texture> texture) 
		: m_uniformBuffer{ device }, m_texture{ texture }, m_sampler{ device }
	{
		assert(m_texture != nullptr && "Texture is null");

		m_descriptorSet = DescriptorSet::Builder(device, pool, setLayout)
			.addBuffer(0, m_uniformBuffer)
			.addTexture(1, *m_texture, m_sampler)
			.build();
	}

	void DefaultMaterialInstance::setData(const DefaultMaterial::Data& data)
	{
		m_uniformBuffer.setData(data);
	}

	DefaultRenderSystem::DefaultRenderSystem(VulkanDevice& device, VkDescriptorSetLayout globalSetLayout)
		: RenderSystem(device, globalSetLayout)
	{
		m_descriptorSetLayout = DescriptorSetLayout::Builder(m_device)
			.addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
			.addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
			.build();

		m_pipelineLayout = PipelineLayout::Builder(m_device)
			.addDescriptorSetLayout(globalSetLayout)
			.addDescriptorSetLayout(*m_descriptorSetLayout)
			.addPushConstantRange(VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(PushConstantData))
			.build();

		m_pipeline = Pipeline::Builder(m_device, *m_pipelineLayout)
			.addShaderStage(VK_SHADER_STAGE_VERTEX_BIT, SHADER_DIR "default.vert.spv")
			.addShaderStage(VK_SHADER_STAGE_FRAGMENT_BIT, SHADER_DIR "default.frag.spv")
			.addColorAttachment(VK_FORMAT_B8G8R8A8_SRGB)
			.setDepthAttachment(VK_FORMAT_D32_SFLOAT)
			.build();
	}

	void DefaultRenderSystem::render(const FrameInfo& frameInfo)
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

		DefaultMaterialInstance* lastMaterial = nullptr;
		auto view = frameInfo.scene.viewEntities<Component::Transform, Component::Mesh, DefaultMaterial>();
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
