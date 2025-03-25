#include "pch.h"

#include "post_processing_pass.h"

#include "graphics/vulkan_tools.h"

#include <imgui.h>

namespace Aegix::Graphics
{
	PostProcessingPass::PostProcessingPass(VulkanDevice& device, DescriptorPool& pool)
	{
		m_descriptorSetLayout = DescriptorSetLayout::Builder{ device }
			.addBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT)
			.addBinding(1, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT)
			.addBinding(2, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT)
			.build();

		m_descriptorSet = std::make_unique<DescriptorSet>(pool, *m_descriptorSetLayout);

		m_pipeline = Pipeline::ComputeBuilder{ device }
			.addDescriptorSetLayout(*m_descriptorSetLayout)
			.addPushConstantRange(VK_SHADER_STAGE_COMPUTE_BIT, sizeof(PostProcessingSettings))
			.setShaderStage(SHADER_DIR "post_process.comp.spv")
			.buildUnique();
	}

	auto PostProcessingPass::createInfo(FrameGraphResourceBuilder& builder) -> FrameGraphNodeCreateInfo
	{
		m_sceneColor = builder.add({
			"SceneColor",
			FrameGraphResourceType::Reference,
			FrameGraphResourceUsage::Compute
			});

		m_bloom = builder.add({
			"Bloom",
			FrameGraphResourceType::Reference,
			FrameGraphResourceUsage::Compute,
			});

		m_final = builder.add({
			"Final",
			FrameGraphResourceType::Texture,
			FrameGraphResourceUsage::Compute,
			FrameGraphResourceTextureInfo{
				.format = VK_FORMAT_B8G8R8A8_UNORM,
				.extent = { 0, 0},
				.resizePolicy = ResizePolicy::SwapchainRelative
				}
			});

		return FrameGraphNodeCreateInfo{
			.name = "Post Processing",
			.inputs = { m_sceneColor, m_bloom },
			.outputs = { m_final }
		};
	}

	void PostProcessingPass::execute(FrameGraphResourcePool& resources, const FrameInfo& frameInfo)
	{
		VkCommandBuffer cmd = frameInfo.commandBuffer;

		DescriptorWriter{ *m_descriptorSetLayout }
			.writeImage(0, resources.texture(m_final))
			.writeImage(1, resources.texture(m_sceneColor))
			.writeImage(2, resources.texture(m_bloom))
			.build(m_descriptorSet->descriptorSet(frameInfo.frameIndex));

		m_pipeline->bind(cmd);
		m_descriptorSet->bind(cmd, m_pipeline->layout(), frameInfo.frameIndex, VK_PIPELINE_BIND_POINT_COMPUTE);

		Tools::vk::cmdPushConstants(cmd, m_pipeline->layout(), VK_SHADER_STAGE_COMPUTE_BIT, m_settings);
		Tools::vk::cmdDispatch(cmd, frameInfo.swapChainExtent, { 16, 16 });
	}

	void PostProcessingPass::drawUI()
	{
		static constexpr auto toneMappingNames = std::array{ "Reinhard", "ACES" };
		int currentMode = static_cast<int>(m_settings.toneMappingMode);
		if (ImGui::Combo("Tone Mapping Mode", &currentMode, toneMappingNames.data(), toneMappingNames.size()))
		{
			m_settings.toneMappingMode = static_cast<ToneMappingMode>(currentMode);
		}

		ImGui::SliderFloat("Bloom Intensity", &m_settings.bloomIntensity, 0.0f, 2.0f);
		ImGui::SliderFloat("Exposure", &m_settings.exposure, 0.1f, 10.0f);
		ImGui::SliderFloat("Gamma", &m_settings.gamma, 1.0f, 5.0f);
	}
}