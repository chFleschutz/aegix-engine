#include "default_render_system.h"

#include "scene/components.h"

namespace Aegix::Graphics
{
	DefaultMaterialInstance::DefaultMaterialInstance(VulkanDevice& device, DescriptorSetLayout& setLayout, DescriptorPool& pool,
		std::shared_ptr<Texture> texture)
		: m_uniformBuffer{ device, setLayout, pool }, m_texture{ texture }
	{
		assert(m_texture != nullptr && "Texture is null");

		for (int i = 0; i < SwapChain::MAX_FRAMES_IN_FLIGHT; i++)
		{
			auto bufferInfo = m_uniformBuffer.descriptorInfo(i);
			auto imageInfo = m_texture->descriptorImageInfo();
			DescriptorWriter(setLayout, pool)
				.writeBuffer(0, &bufferInfo)
				.writeImage(1, &imageInfo)
				.build(m_descriptorSets[i]);
		}
	}

	void DefaultMaterialInstance::setData(const DefaultMaterial::Data& data)
	{
		m_data = data;
		m_uniformBuffer.setData(data);
	}

	DefaultRenderSystem::DefaultRenderSystem(VulkanDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout)
		: RenderSystem(device)
	{
		m_descriptorSetLayout = DescriptorSetLayout::Builder(m_device)
			.addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
			.addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
			.build();

		m_pipelineLayout = PipelineLayout::Builder(m_device)
			.addDescriptorSetLayout(globalSetLayout)
			.addDescriptorSetLayout(m_descriptorSetLayout->descriptorSetLayout())
			.addPushConstantRange(VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(PushConstantData))
			.build();

		m_pipeline = Pipeline::Builder(m_device)
			.setRenderPass(renderPass)
			.setPipelineLayout(m_pipelineLayout->pipelineLayout())
			.addShaderStage(VK_SHADER_STAGE_VERTEX_BIT, SHADER_DIR "default.vert.spv")
			.addShaderStage(VK_SHADER_STAGE_FRAGMENT_BIT, SHADER_DIR "default.frag.spv")
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
		auto view = frameInfo.scene->viewEntities<Component::Transform, Component::Mesh, DefaultMaterial>();
		for (auto&& [entity, transform, mesh, material] : view.each())
		{
			if (mesh.staticMesh == nullptr || material.instance == nullptr)
				continue;
			
			// Material Descriptor Set
			if (lastMaterial != material.instance.get())
			{
				lastMaterial = material.instance.get();
				vkCmdBindDescriptorSets(
					frameInfo.commandBuffer,
					VK_PIPELINE_BIND_POINT_GRAPHICS,
					m_pipelineLayout->pipelineLayout(),
					1, 1,
					&material.instance->m_descriptorSets[frameInfo.frameIndex],
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
