#pragma once

#include "graphics/frame_graph/frame_graph_render_pass.h"
#include "graphics/texture.h"
#include "graphics/vulkan_tools.h"

namespace Aegix::Graphics
{
	class BloomPass : public FrameGraphRenderPass
	{
	public:
		static constexpr uint32_t BLOOM_MIP_LEVELS = 6;

		BloomPass(VulkanDevice& device, DescriptorPool& pool)
			: m_sampler{ device, VK_FILTER_LINEAR, VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, false }
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


			m_upsampleSetLayout = DescriptorSetLayout::Builder{ device }
				.addBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT)
				.addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_COMPUTE_BIT)
				.build();

			for (uint32_t i = 0; i < BLOOM_MIP_LEVELS - 1; i++)
			{
				m_upsampleSets.emplace_back(std::make_unique<DescriptorSet>(pool, *m_upsampleSetLayout));
			}

			m_upsamplePipelineLayout = PipelineLayout::Builder{ device }
				.addDescriptorSetLayout(*m_upsampleSetLayout)
				.build();

			m_upsamplePipeline = Pipeline::ComputeBuilder{ device, *m_upsamplePipelineLayout }
				.setShaderStage(SHADER_DIR "bloom_upsample.comp.spv")
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

			for (uint32_t i = 0; i < BLOOM_MIP_LEVELS - 1; i++)
			{
				DescriptorWriter{ *m_upsampleSetLayout }
					.writeImage(0, VkDescriptorImageInfo{ VK_NULL_HANDLE, m_mipViews[i], VK_IMAGE_LAYOUT_GENERAL })
					.writeImage(1, VkDescriptorImageInfo{ m_sampler, m_mipViews[i + 1], VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL})
					.build(m_upsampleSets[i]->descriptorSet(0));
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
			bloom.generateMipmaps(cmd, VK_IMAGE_LAYOUT_GENERAL);

			// Upsample
			m_upsamplePipeline->bind(cmd);

			VkImageMemoryBarrier barrier{};
			barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			barrier.image = bloom.image();
			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			barrier.subresourceRange.baseArrayLayer = 0;
			barrier.subresourceRange.layerCount = 1;

			for (uint32_t i = BLOOM_MIP_LEVELS - 1; i > 0; i--)
			{
				uint32_t srcMip = i;
				uint32_t dstMip = i - 1;

				barrier.subresourceRange.baseMipLevel = srcMip;
				barrier.subresourceRange.levelCount = 1;
				barrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
				barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
				barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

				vkCmdPipelineBarrier(cmd,
					VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 0,
					0, nullptr,
					0, nullptr,
					1, &barrier
				);

				m_upsampleSets[dstMip]->bind(cmd, *m_upsamplePipelineLayout, 0, VK_PIPELINE_BIND_POINT_COMPUTE);

				VkExtent2D mipExtent = { bloom.width() >> dstMip, bloom.height() >> dstMip };
				Tools::vk::cmdDispatch(cmd, mipExtent, { 16, 16 });
			}

			barrier.subresourceRange.baseMipLevel = 1;
			barrier.subresourceRange.levelCount = BLOOM_MIP_LEVELS - 1;
			barrier.oldLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
			barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT;

			vkCmdPipelineBarrier(cmd,
				VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 0,
				0, nullptr,
				0, nullptr,
				1, &barrier
			);
		}

	private:
		FrameGraphResourceHandle m_sceneColor;
		FrameGraphResourceHandle m_bloom;
		std::vector<ImageView> m_mipViews;
		Sampler m_sampler;

		std::unique_ptr<Pipeline> m_thresholdPipeline;
		std::unique_ptr<PipelineLayout> m_thresholdPipelineLayout;
		std::unique_ptr<DescriptorSetLayout> m_thresholdSetLayout;
		std::unique_ptr<DescriptorSet> m_thresholdSet;

		std::unique_ptr<Pipeline> m_upsamplePipeline;
		std::unique_ptr<PipelineLayout> m_upsamplePipelineLayout;
		std::unique_ptr<DescriptorSetLayout> m_upsampleSetLayout;
		std::vector<std::unique_ptr<DescriptorSet>> m_upsampleSets;
	};
}