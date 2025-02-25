#pragma once

#include "graphics/frame_graph/frame_graph_render_pass.h"
#include "graphics/descriptors.h"

#include <array>

namespace Aegix::Graphics
{
	class PostProcessingPass : public FrameGraphRenderPass
	{
	public:
		PostProcessingPass(VulkanDevice& device, DescriptorPool& pool)
		{
			m_descriptorSetLayout = DescriptorSetLayout::Builder{ device }
				.addBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT)
				.addBinding(1, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT)
				.build();

			m_descriptorSet = std::make_unique<DescriptorSet>(pool, *m_descriptorSetLayout);

			m_pipelineLayout = PipelineLayout::Builder{ device }
				.addDescriptorSetLayout(*m_descriptorSetLayout)
				.build();

			m_pipeline = Pipeline::ComputeBuilder{ device, *m_pipelineLayout }
				.setShaderStage(SHADER_DIR "post_process.comp.spv")
				.build();
		}

		virtual auto createInfo(FrameGraphResourceBuilder& builder) -> FrameGraphNodeCreateInfo override
		{
			m_sceneColor = builder.add({
				"SceneColor",
				FrameGraphResourceType::Reference,
				FrameGraphResourceUsage::Compute
				});

			m_final = builder.add({
				"Final",
				FrameGraphResourceType::Texture,
				FrameGraphResourceUsage::Compute,
				FrameGraphResourceTextureInfo{
					.format = VK_FORMAT_R16G16B16A16_SFLOAT,
					.extent = { 0, 0},
					.resizePolicy = ResizePolicy::SwapchainRelative
					}
				});

			return FrameGraphNodeCreateInfo{
				.name = "Post Processing Pass",
				.inputs = { m_sceneColor },
				.outputs = { m_final }
			};
		}

		virtual void execute(FrameGraphResourcePool& resources, const FrameInfo& frameInfo) override
		{
			auto& sceneColor = resources.texture(m_sceneColor);
			auto& final = resources.texture(m_final);

			auto sceneColorInfo = sceneColor.descriptorImageInfo();
			sceneColorInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
			auto finalInfo = final.descriptorImageInfo();
			finalInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

			DescriptorWriter{ *m_descriptorSetLayout }
				.writeImage(0, &sceneColorInfo)
				.writeImage(1, &finalInfo)
				.build(m_descriptorSet->descriptorSet(frameInfo.frameIndex));

			VkCommandBuffer cmd = frameInfo.commandBuffer;

			m_pipeline->bind(cmd);
			m_descriptorSet->bind(cmd, *m_pipelineLayout, frameInfo.frameIndex, VK_PIPELINE_BIND_POINT_COMPUTE);

			uint32_t groupCountX = (final.extent().width + 15) / 16;
			uint32_t groupCountY = (final.extent().height + 15) / 16;
			vkCmdDispatch(cmd, groupCountX, groupCountY, 1);
		}

	private:
		std::unique_ptr<DescriptorSetLayout> m_descriptorSetLayout;
		std::unique_ptr<DescriptorSet> m_descriptorSet;
		std::unique_ptr<PipelineLayout> m_pipelineLayout;
		std::unique_ptr<Pipeline> m_pipeline;

		FrameGraphResourceHandle m_sceneColor;
		FrameGraphResourceHandle m_final;
	};
}