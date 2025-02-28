#pragma once

#include "graphics/descriptors.h"
#include "graphics/frame_graph/frame_graph_render_pass.h"
#include "graphics/vulkan_tools.h"

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
				.addBinding(2, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT)
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
					.format = VK_FORMAT_R16G16B16A16_SFLOAT,
					.extent = { 0, 0},
					.resizePolicy = ResizePolicy::SwapchainRelative
					}
				});

			return FrameGraphNodeCreateInfo{
				.name = "Post Processing Pass",
				.inputs = { m_sceneColor, m_bloom },
				.outputs = { m_final }
			};
		}

		virtual void execute(FrameGraphResourcePool& resources, const FrameInfo& frameInfo) override
		{
			VkCommandBuffer cmd = frameInfo.commandBuffer;

			DescriptorWriter{ *m_descriptorSetLayout }
				.writeImage(0, resources.texture(m_final))
				.writeImage(1, resources.texture(m_sceneColor))
				.writeImage(2, resources.texture(m_bloom))
				.build(m_descriptorSet->descriptorSet(frameInfo.frameIndex));

			m_pipeline->bind(cmd);
			m_descriptorSet->bind(cmd, *m_pipelineLayout, frameInfo.frameIndex, VK_PIPELINE_BIND_POINT_COMPUTE);

			Tools::vk::cmdDispatch(cmd, frameInfo.swapChainExtent, { 16, 16 });
		}

	private:
		std::unique_ptr<DescriptorSetLayout> m_descriptorSetLayout;
		std::unique_ptr<DescriptorSet> m_descriptorSet;
		std::unique_ptr<PipelineLayout> m_pipelineLayout;
		std::unique_ptr<Pipeline> m_pipeline;

		FrameGraphResourceHandle m_sceneColor;
		FrameGraphResourceHandle m_bloom;
		FrameGraphResourceHandle m_final;
	};
}