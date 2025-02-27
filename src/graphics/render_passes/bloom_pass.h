#pragma once

#include "graphics/frame_graph/frame_graph_render_pass.h"
#include "graphics/vulkan_tools.h"

namespace Aegix::Graphics
{
	class BloomPass : public FrameGraphRenderPass
	{
	public:
		static constexpr uint32_t BLOOM_MIP_LEVELS = 6;

		BloomPass(VulkanDevice& device, DescriptorPool& pool)
		{
			for (uint32_t i = 0; i < BLOOM_MIP_LEVELS; i++)
			{
				m_mipViews.emplace_back(device);
			}

			m_thresholdSetLayout = DescriptorSetLayout::Builder{ device }
				.addBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT)
				.addBinding(1, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT)
				.build();

			m_thresholdSet = std::make_unique<DescriptorSet>(pool, *m_thresholdSetLayout);

			m_thresholdPipelineLayout = PipelineLayout::Builder{ device }
				.addDescriptorSetLayout(*m_thresholdSetLayout)
				.build();

			m_thresholdPipeline = Pipeline::ComputeBuilder{ device, *m_thresholdPipelineLayout }
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

			m_bloom = builder.add({
				"Bloom",
				FrameGraphResourceType::Texture,
				FrameGraphResourceUsage::Compute,
				FrameGraphResourceTextureInfo{
					.format = VK_FORMAT_R16G16B16A16_SFLOAT,
					.extent = { 0, 0 },
					.resizePolicy = ResizePolicy::SwapchainRelative,
					.usage = VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
					.mipLevels = BLOOM_MIP_LEVELS
					}
				});

			return FrameGraphNodeCreateInfo{
				.name = "Bloom",
				.inputs = { m_sceneColor },
				.outputs = { m_bloom },
			};
		}

		virtual void createResources(FrameGraphResourcePool& resources) override
		{
			auto& bloom = resources.texture(m_bloom);
			for (uint32_t i = 0; i < BLOOM_MIP_LEVELS; i++)
			{
				m_mipViews[i] = ImageView{ bloom, i, 1 };
			}
		}

		virtual void execute(FrameGraphResourcePool& resources, const FrameInfo& frameInfo) override
		{
			auto& sceneColor = resources.texture(m_sceneColor);
			auto& bloom = resources.texture(m_bloom);

			VkCommandBuffer cmd = frameInfo.commandBuffer;

			// Extract bright regions
			DescriptorWriter{ *m_thresholdSetLayout }
				.writeImage(0, sceneColor)
				.writeImage(1, VkDescriptorImageInfo{ VK_NULL_HANDLE, m_mipViews[0], bloom.layout() })
				.build(m_thresholdSet->descriptorSet(frameInfo.frameIndex));

			m_thresholdPipeline->bind(cmd);
			m_thresholdSet->bind(cmd, *m_thresholdPipelineLayout, frameInfo.frameIndex, VK_PIPELINE_BIND_POINT_COMPUTE);

			Tools::vk::cmdDispatch(cmd, frameInfo.swapChainExtent, { 16, 16 });

			// Downsample
			bloom.generateMipmaps(cmd, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

			// Upsample

		}

	private:
		FrameGraphResourceHandle m_sceneColor;
		FrameGraphResourceHandle m_bloom;
		std::vector<ImageView> m_mipViews;

		std::unique_ptr<Pipeline> m_thresholdPipeline;
		std::unique_ptr<PipelineLayout> m_thresholdPipelineLayout;
		std::unique_ptr<DescriptorSetLayout> m_thresholdSetLayout;
		std::unique_ptr<DescriptorSet> m_thresholdSet;
	};
}