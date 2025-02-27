#pragma once

#include "graphics/frame_graph/frame_graph_render_pass.h"
#include "graphics/vulkan_tools.h"

namespace Aegix::Graphics
{
	class BloomPass : public FrameGraphRenderPass
	{
	public:
		BloomPass(VulkanDevice& device, DescriptorPool& pool)
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
				.setShaderStage(SHADER_DIR "bloom_threshold.comp.spv")
				.build();

		}

		virtual auto createInfo(FrameGraphResourceBuilder& builder) -> FrameGraphNodeCreateInfo override
		{
			m_sceneColor = builder.add({
				"SceneColor",
				FrameGraphResourceType::Reference,
				FrameGraphResourceUsage::Compute
				});

			m_bloomMips = builder.add({
				"BloomMips",
				FrameGraphResourceType::Texture,
				FrameGraphResourceUsage::Compute,
				FrameGraphResourceTextureInfo{
					.format = VK_FORMAT_R16G16B16A16_SFLOAT,
					.extent = { 0, 0 },
					.resizePolicy = ResizePolicy::SwapchainRelative,
					.usage = VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | 
						VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
					.mipmapping = true
					}
				});

			m_bloom = builder.add({
				"Bloom",
				FrameGraphResourceType::Texture,
				FrameGraphResourceUsage::Compute,
				FrameGraphResourceTextureInfo{
					.format = VK_FORMAT_R16G16B16A16_SFLOAT,
					.extent = { 0, 0 },
					.resizePolicy = ResizePolicy::SwapchainRelative,
					}
				});


			return FrameGraphNodeCreateInfo{
				.name = "Bloom",
				.inputs = { m_sceneColor },
				.outputs = { m_bloom, m_bloomMips },
			};
		}

		virtual void execute(FrameGraphResourcePool& resources, const FrameInfo& frameInfo) override
		{
			auto& sceneColor = resources.texture(m_sceneColor);
			auto& bloomMips = resources.texture(m_bloomMips);
			auto& bloom = resources.texture(m_bloom);

			VkCommandBuffer cmd = frameInfo.commandBuffer;

			// Extract bright regions
			DescriptorWriter{ *m_descriptorSetLayout }
				.writeImage(0, sceneColor)
				.writeImage(1, bloomMips)
				.build(m_descriptorSet->descriptorSet(frameInfo.frameIndex));

			m_pipeline->bind(cmd);
			m_descriptorSet->bind(cmd, *m_pipelineLayout, frameInfo.frameIndex, VK_PIPELINE_BIND_POINT_COMPUTE);

			Tools::vk::cmdDispatch(cmd, frameInfo.swapChainExtent, { 16, 16 });

			// Downsample
			bloomMips.generateMipmaps(cmd, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

			// Upsample

			// Combine (in post processing pass)
		}

	private:
		FrameGraphResourceHandle m_sceneColor;
		FrameGraphResourceHandle m_bloomMips;
		FrameGraphResourceHandle m_bloom;

		std::unique_ptr<Pipeline> m_pipeline;
		std::unique_ptr<PipelineLayout> m_pipelineLayout;
		std::unique_ptr<DescriptorSetLayout> m_descriptorSetLayout;
		std::unique_ptr<DescriptorSet> m_descriptorSet;
	};
}