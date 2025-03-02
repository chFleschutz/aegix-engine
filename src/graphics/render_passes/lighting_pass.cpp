#include "lighting_pass.h"

#include "graphics/vulkan_tools.h"

#include <imgui.h>

namespace Aegix::Graphics
{
	LightingPass::LightingPass(VulkanDevice& device, DescriptorPool& pool)
	{
		m_descriptorSetLayout = DescriptorSetLayout::Builder(device)
			.addBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT)
			.addBinding(1, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT)
			.addBinding(2, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT)
			.addBinding(3, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT)
			.addBinding(4, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT)
			.addBinding(5, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT)
			.addBinding(6, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT)
			.build();

		m_descriptorSet = std::make_unique<DescriptorSet>(pool, *m_descriptorSetLayout);

		m_ubo = std::make_unique<UniformBuffer>(device, m_lighting);

		m_pipelineLayout = PipelineLayout::Builder(device)
			.addDescriptorSetLayout(*m_descriptorSetLayout)
			.build();

		m_pipeline = Pipeline::ComputeBuilder(device, *m_pipelineLayout)
			.setShaderStage(SHADER_DIR "lighting.comp.spv")
			.build();
	}

	auto LightingPass::createInfo(FrameGraphResourceBuilder& builder) -> FrameGraphNodeCreateInfo
	{
		m_position = builder.add({ "Position",
			FrameGraphResourceType::Reference,
			FrameGraphResourceUsage::Compute
			});
		m_normal = builder.add({ "Normal",
			FrameGraphResourceType::Reference,
			FrameGraphResourceUsage::Compute
			});
		m_albedo = builder.add({ "Albedo",
			FrameGraphResourceType::Reference,
			FrameGraphResourceUsage::Compute
			});
		m_arm = builder.add({ "ARM",
			FrameGraphResourceType::Reference,
			FrameGraphResourceUsage::Compute
			});
		m_emissive = builder.add({ "Emissive",
			FrameGraphResourceType::Reference,
			FrameGraphResourceUsage::Compute
			});
		m_sceneColor = builder.add({ "SceneColor",
			FrameGraphResourceType::Texture,
			FrameGraphResourceUsage::Compute,
			FrameGraphResourceTextureInfo{
				.format = VK_FORMAT_R16G16B16A16_SFLOAT,
				.extent = { 0, 0 },
				.resizePolicy = ResizePolicy::SwapchainRelative
				}
			});

		return FrameGraphNodeCreateInfo{
			.name = "Lighting",
			.inputs = { m_position, m_normal, m_albedo, m_arm, m_emissive },
			.outputs = { m_sceneColor }
		};
	}

	void LightingPass::execute(FrameGraphResourcePool& resources, const FrameInfo& frameInfo)
	{
		VkCommandBuffer cmd = frameInfo.commandBuffer;

		updateLightingUBO(frameInfo);

		DescriptorWriter{ *m_descriptorSetLayout }
			.writeImage(0, resources.texture(m_sceneColor))
			.writeImage(1, resources.texture(m_position))
			.writeImage(2, resources.texture(m_normal))
			.writeImage(3, resources.texture(m_albedo))
			.writeImage(4, resources.texture(m_arm))
			.writeImage(5, resources.texture(m_emissive))
			.writeBuffer(6, m_ubo->descriptorBufferInfo(frameInfo.frameIndex))
			.build(m_descriptorSet->descriptorSet(frameInfo.frameIndex));

		m_pipeline->bind(cmd);
		m_descriptorSet->bind(cmd, *m_pipelineLayout, frameInfo.frameIndex, VK_PIPELINE_BIND_POINT_COMPUTE);

		Tools::vk::cmdDispatch(cmd, frameInfo.swapChainExtent, { 16, 16 });
	}

	void LightingPass::drawUI()
	{
		ImGui::Text("Ambient Light");
		ImGui::ColorEdit3("Color##Ambient", &m_lighting.ambient.color.r);
		ImGui::DragFloat("Intensity##Ambient", &m_lighting.ambient.color.a, 0.01f, 0.0f, 10.0f);

		ImGui::Spacing();

		ImGui::Text("Directional Light");
		ImGui::ColorEdit3("Color##Directional", &m_lighting.directional.color.r);
		ImGui::DragFloat("Intensity##Directional", &m_lighting.directional.color.a, 0.1f, 0.0f, 100.0f);
		ImGui::DragFloat3("Direction##Directional", &m_lighting.directional.direction.x, 0.01f);

		ImGui::Spacing();
	}

	void LightingPass::updateLightingUBO(const FrameInfo& frameInfo)
	{
		m_lighting.cameraPosition = glm::vec4(frameInfo.scene.camera().component<Component::Transform>().location, 1.0f);

		int32_t lighIndex = 0;
		auto view = frameInfo.scene.viewEntities<Aegix::Component::Transform, Aegix::Component::PointLight>();
		for (auto&& [entity, transform, pointLight] : view.each())
		{
			assert(lighIndex < MAX_POINT_LIGHTS && "Point lights exceed maximum number of point lights");
			m_lighting.pointLights[lighIndex] = LightingUniforms::PointLight{
				.position = glm::vec4(transform.location, 1.0f),
				.color = glm::vec4(pointLight.color, pointLight.intensity)
			};
			lighIndex++;
		}
		m_lighting.pointLightCount = lighIndex;

		m_ubo->setData(frameInfo.frameIndex, m_lighting);
	}
}