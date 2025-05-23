#include "pch.h"

#include "default_render_system.h"

#include "core/globals.h"
#include "graphics/vulkan_context.h"
#include "graphics/vulkan_tools.h"
#include "math/math.h"

namespace Aegix::Graphics
{
	DefaultMaterialInstance::DefaultMaterialInstance(DescriptorSetLayout& setLayout, 
		std::shared_ptr<Texture> albedo, std::shared_ptr<Texture> normal, std::shared_ptr<Texture> metalRoughness,
		std::shared_ptr<Texture> ao, std::shared_ptr<Texture> emissive, DefaultMaterial::Data data)
		: m_albedoTexture{ albedo }, m_normalTexture{ normal },	m_metalRoughnessTexture{ metalRoughness },
		m_aoTexture{ ao }, m_emissiveTexture{ emissive }, 
		m_uniformBuffer{ Buffer::createUniformBuffer(sizeof(DefaultMaterial::Data)) }
	{
		m_uniformBuffer.singleWrite(&data);

		m_descriptorSet = DescriptorSet::Builder(VulkanContext::descriptorPool(), setLayout)
			.addBuffer(0, m_uniformBuffer)
			.addTexture(1, m_albedoTexture)
			.addTexture(2, m_normalTexture)
			.addTexture(3, m_metalRoughnessTexture)
			.addTexture(4, m_aoTexture)
			.addTexture(5, m_emissiveTexture)
			.build();
	}

	DefaultRenderSystem::DefaultRenderSystem(VkDescriptorSetLayout globalSetLayout)
		: RenderSystem(globalSetLayout)
	{
		m_descriptorSetLayout = DescriptorSetLayout::Builder{}
			.addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
			.addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
			.addBinding(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
			.addBinding(3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
			.addBinding(4, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
			.addBinding(5, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
			.build();

		m_pipeline = Pipeline::GraphicsBuilder{}
			.addDescriptorSetLayout(globalSetLayout)
			.addDescriptorSetLayout(*m_descriptorSetLayout)
			.addPushConstantRange(VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(PushConstantData))
			.addShaderStage(VK_SHADER_STAGE_VERTEX_BIT, SHADER_DIR "default_geometry.vert.spv")
			.addShaderStage(VK_SHADER_STAGE_FRAGMENT_BIT, SHADER_DIR "default_geometry.frag.spv")
			.addColorAttachment(VK_FORMAT_R16G16B16A16_SFLOAT)
			.addColorAttachment(VK_FORMAT_R16G16B16A16_SFLOAT)
			.addColorAttachment(VK_FORMAT_R8G8B8A8_UNORM)
			.addColorAttachment(VK_FORMAT_R8G8B8A8_UNORM)
			.addColorAttachment(VK_FORMAT_R8G8B8A8_UNORM)
			.setDepthAttachment(VK_FORMAT_D32_SFLOAT)
			.buildUnique();
	}

	void DefaultRenderSystem::render(const FrameInfo& frameInfo, VkDescriptorSet globalSet)
	{
		VkCommandBuffer cmd = frameInfo.commandBuffer;

		m_pipeline->bind(cmd);
		m_pipeline->bindDescriptorSet(cmd, 0, globalSet);

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

				m_pipeline->bindDescriptorSet(cmd, 1, descriptorSet->descriptorSet(frameInfo.frameIndex));
			}

			// Push Constants
			PushConstantData push{
				.modelMatrix = globalTransform.matrix(),
				.normalMatrix = Math::normalMatrix(globalTransform.rotation, globalTransform.scale)
			};
			m_pipeline->pushConstants(cmd, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, push);
			
			// Draw
			mesh.staticMesh->bind(cmd);
			mesh.staticMesh->draw(cmd);
		}
	}
}