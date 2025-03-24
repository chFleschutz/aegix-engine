#include "pch.h"

#include "default_render_system.h"

#include "graphics/vulkan_tools.h"
#include "math/math.h"

namespace Aegix::Graphics
{
	DefaultMaterialInstance::DefaultMaterialInstance(VulkanDevice& device, DescriptorSetLayout& setLayout, DescriptorPool& pool,
		std::shared_ptr<Texture> albedo, std::shared_ptr<Texture> normal, std::shared_ptr<Texture> metalRoughness,
		std::shared_ptr<Texture> ao, std::shared_ptr<Texture> emissive, DefaultMaterial::Data data)
		: m_albedoTexture{ albedo }, m_normalTexture{ normal },	m_metalRoughnessTexture{ metalRoughness },
		m_aoTexture{ ao }, m_emissiveTexture{ emissive }, 
		m_uniformBuffer{ Buffer::createUniformBuffer(device, sizeof(DefaultMaterial::Data)) }
	{
		//for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		//{
		//	m_uniformBuffer.writeToIndex(&data, i);
		//}

		m_uniformBuffer.singleWrite(&data);

		m_descriptorSet = DescriptorSet::Builder(device, pool, setLayout)
			.addBuffer(0, m_uniformBuffer)
			.addTexture(1, m_albedoTexture)
			.addTexture(2, m_normalTexture)
			.addTexture(3, m_metalRoughnessTexture)
			.addTexture(4, m_aoTexture)
			.addTexture(5, m_emissiveTexture)
			.build();
	}

	DefaultRenderSystem::DefaultRenderSystem(VulkanDevice& device, VkDescriptorSetLayout globalSetLayout)
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

		m_pipeline = Pipeline::GraphicsBuilder(m_device, *m_pipelineLayout)
			.addShaderStage(VK_SHADER_STAGE_VERTEX_BIT, SHADER_DIR "default_geometry.vert.spv")
			.addShaderStage(VK_SHADER_STAGE_FRAGMENT_BIT, SHADER_DIR "default_geometry.frag.spv")
			.addColorAttachment(VK_FORMAT_R16G16B16A16_SFLOAT)
			.addColorAttachment(VK_FORMAT_R16G16B16A16_SFLOAT)
			.addColorAttachment(VK_FORMAT_R8G8B8A8_UNORM)
			.addColorAttachment(VK_FORMAT_R8G8B8A8_UNORM)
			.addColorAttachment(VK_FORMAT_R8G8B8A8_UNORM)
			.setDepthAttachment(VK_FORMAT_D32_SFLOAT)
			.build();
	}

	void DefaultRenderSystem::render(const FrameInfo& frameInfo, VkDescriptorSet globalSet)
	{
		VkCommandBuffer cmd = frameInfo.commandBuffer;

		m_pipeline->bind(cmd);

		// Global Descriptor Set
		Tools::vk::cmdBindDescriptorSet(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
			*m_pipelineLayout, globalSet);

		DefaultMaterialInstance* lastMaterial = nullptr;
		auto view = frameInfo.scene.registry().view<GlobalTransform, Mesh, DefaultMaterial>();
		for (auto&& [entity, globalTransform, mesh, material] : view.each())
		{
			if (mesh.staticMesh == nullptr || material.instance == nullptr)
				continue;

			// Material Descriptor Set
			if (lastMaterial != material.instance.get())
			{
				lastMaterial = material.instance.get();
				auto& descriptorSet = material.instance->m_descriptorSet;
				AGX_ASSERT_X(descriptorSet != nullptr, "Material descriptor set is null");

				Tools::vk::cmdBindDescriptorSet(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
					*m_pipelineLayout, descriptorSet->descriptorSet(frameInfo.frameIndex), 1);
			}

			// Push Constants
			PushConstantData push{};
			push.modelMatrix = globalTransform.matrix();
			push.normalMatrix = Math::normalMatrix(globalTransform.rotation, globalTransform.scale);

			Tools::vk::cmdPushConstants(cmd, *m_pipelineLayout,
				VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, push);
			
			// Draw
			mesh.staticMesh->bind(cmd);
			mesh.staticMesh->draw(cmd);
		}
	}
}