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
			.addBinding(6, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_COMPUTE_BIT)
			.addBinding(7, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_COMPUTE_BIT)
			.addBinding(8, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT)
			.build();

		m_descriptorSet = std::make_unique<DescriptorSet>(pool, *m_descriptorSetLayout);

		m_ubo = std::make_unique<UniformBuffer>(device, LightingUniforms{});

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
		m_ssao = builder.add({ "SSAO",
			FrameGraphResourceType::Reference,
			FrameGraphResourceUsage::Sampled
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
			.inputs = { m_position, m_normal, m_albedo, m_arm, m_emissive, m_ssao },
			.outputs = { m_sceneColor }
		};
	}

	void LightingPass::execute(FrameGraphResourcePool& resources, const FrameInfo& frameInfo)
	{
		VkCommandBuffer cmd = frameInfo.commandBuffer;

		updateLightingUBO(frameInfo);
		auto& environment = frameInfo.scene.environment().component<Environment>();
		assert(environment.irradiance && "Environment irradiance map is not set");

		DescriptorWriter{ *m_descriptorSetLayout }
			.writeImage(0, resources.texture(m_sceneColor))
			.writeImage(1, resources.texture(m_position))
			.writeImage(2, resources.texture(m_normal))
			.writeImage(3, resources.texture(m_albedo))
			.writeImage(4, resources.texture(m_arm))
			.writeImage(5, resources.texture(m_emissive))
			.writeImage(6, resources.texture(m_ssao))
			.writeImage(7, *environment.irradiance)
			.writeBuffer(8, m_ubo->descriptorBufferInfo(frameInfo.frameIndex))
			.build(m_descriptorSet->descriptorSet(frameInfo.frameIndex));

		m_pipeline->bind(cmd);
		m_descriptorSet->bind(cmd, *m_pipelineLayout, frameInfo.frameIndex, VK_PIPELINE_BIND_POINT_COMPUTE);

		Tools::vk::cmdDispatch(cmd, frameInfo.swapChainExtent, { 16, 16 });
	}

	void LightingPass::drawUI()
	{
		ImGui::DragFloat("AO Factor", &m_ambientOcclusionFactor, 0.01f, 0.0f, 1.0f);

		static constexpr auto viewModeNames = std::array{ "Scene Color", "Albedo", "Ambient Occlusion" , "Roughness", 
			"Metallic",	"Emissive" };
		int currentMode = static_cast<int>(m_viewMode);
		if (ImGui::Combo("View Mode", &currentMode, viewModeNames.data(), viewModeNames.size()))
		{
			m_viewMode = static_cast<LightingViewMode>(currentMode);
		}
	}

	void LightingPass::updateLightingUBO(const FrameInfo& frameInfo)
	{
		LightingUniforms lighting;

		Scene::Entity mainCamera = frameInfo.scene.mainCamera();
		if (mainCamera && mainCamera.hasComponent<Transform>())
		{
			auto& cameraTransform = mainCamera.component<Transform>();
			lighting.cameraPosition = glm::vec4(cameraTransform.location, 1.0f);
		}

		Scene::Entity ambientLight = frameInfo.scene.ambientLight();
		if (ambientLight && ambientLight.hasComponent<AmbientLight>())
		{
			auto& ambient = ambientLight.component<AmbientLight>();
			lighting.ambient.color = glm::vec4(ambient.color, ambient.intensity);
		}

		Scene::Entity directionalLight = frameInfo.scene.directionalLight();
		if (directionalLight && directionalLight.hasComponent<DirectionalLight, Transform>())
		{
			auto& directional = directionalLight.component<DirectionalLight>();
			auto& transform = directionalLight.component<Transform>();
			lighting.directional.color = glm::vec4(directional.color, directional.intensity);
			lighting.directional.direction = glm::vec4(glm::normalize(transform.forward()), 0.0f);
		}

		int32_t lighIndex = 0;
		auto view = frameInfo.scene.registry().view<Transform, PointLight>();
		for (auto&& [entity, transform, pointLight] : view.each())
		{
			assert(lighIndex < MAX_POINT_LIGHTS && "Point lights exceed maximum number of point lights");
			lighting.pointLights[lighIndex] = LightingUniforms::PointLight{
				.position = glm::vec4(transform.location, 1.0f),
				.color = glm::vec4(pointLight.color, pointLight.intensity)
			};
			lighIndex++;
		}
		lighting.pointLightCount = lighIndex;

		lighting.ambientOcclusionFactor = m_ambientOcclusionFactor;
		lighting.viewMode = m_viewMode;

		m_ubo->setData(frameInfo.frameIndex, lighting);
	}
}