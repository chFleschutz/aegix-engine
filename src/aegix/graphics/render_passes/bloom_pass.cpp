#include "pch.h"

#include "bloom_pass.h"

#include "graphics/vulkan_tools.h"
#include "graphics/vulkan_context.h"

#include <imgui.h>

namespace Aegix::Graphics
{
	BloomPass::BloomPass()
	{
		m_sampler.create(VK_FILTER_LINEAR, VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, false);

		for (uint32_t i = 0; i < BLOOM_MIP_LEVELS; i++)
		{
			m_mipViews.emplace_back();
		}

		// Threshold
		m_thresholdSetLayout = DescriptorSetLayout::Builder{}
			.addBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT)
			.addBinding(1, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT)
			.build();

		m_thresholdSet = std::make_unique<DescriptorSet>(*m_thresholdSetLayout);

		m_thresholdPipeline = Pipeline::ComputeBuilder{}
			.addDescriptorSetLayout(*m_thresholdSetLayout)
			.addPushConstantRange(VK_SHADER_STAGE_COMPUTE_BIT, sizeof(BloomThreshold))
			.setShaderStage(SHADER_DIR "bloom_threshold.comp.spv")
			.buildUnique();

		// Downsample
		m_downsampleSetLayout = DescriptorSetLayout::Builder{}
			.addBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT)
			.addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_COMPUTE_BIT)
			.build();

		for (uint32_t i = 0; i < BLOOM_MIP_LEVELS - 1; i++)
		{
			m_downsampleSets.emplace_back(std::make_unique<DescriptorSet>(*m_downsampleSetLayout));
		}

		m_downsamplePipeline = Pipeline::ComputeBuilder{}
			.addDescriptorSetLayout(*m_downsampleSetLayout)
			.addPushConstantRange(VK_SHADER_STAGE_COMPUTE_BIT, sizeof(BloomDownsample))
			.setShaderStage(SHADER_DIR "bloom_downsample.comp.spv")
			.buildUnique();

		// Upsample
		m_upsampleSetLayout = DescriptorSetLayout::Builder{}
			.addBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT)
			.addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_COMPUTE_BIT)
			.build();

		for (uint32_t i = 0; i < BLOOM_MIP_LEVELS - 1; i++)
		{
			m_upsampleSets.emplace_back(std::make_unique<DescriptorSet>(*m_upsampleSetLayout));
		}

		m_upsamplePipeline = Pipeline::ComputeBuilder{}
			.addDescriptorSetLayout(*m_upsampleSetLayout)
			.addPushConstantRange(VK_SHADER_STAGE_COMPUTE_BIT, sizeof(BloomUpsample))
			.setShaderStage(SHADER_DIR "bloom_upsample.comp.spv")
			.buildUnique();
	}

	auto BloomPass::createInfo(FrameGraphResourceBuilder& builder) -> FrameGraphNodeCreateInfo
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

	void BloomPass::createResources(FrameGraphResourcePool& resources)
	{
		auto& bloom = resources.texture(m_bloom);
		auto& sceneColor = resources.texture(m_sceneColor);

		for (uint32_t i = 0; i < BLOOM_MIP_LEVELS; i++)
		{
			m_mipViews[i].create2D(bloom.image(), i, 1);
		}

		DescriptorWriter{ *m_thresholdSetLayout }
			.writeImage(0, VkDescriptorImageInfo{ VK_NULL_HANDLE, sceneColor.view(), VK_IMAGE_LAYOUT_GENERAL })
			.writeImage(1, VkDescriptorImageInfo{ VK_NULL_HANDLE, m_mipViews[0], VK_IMAGE_LAYOUT_GENERAL })
			.build(m_thresholdSet->descriptorSet(0));

		for (uint32_t i = 0; i < BLOOM_MIP_LEVELS - 1; i++)
		{
			DescriptorWriter{ *m_upsampleSetLayout }
				.writeImage(0, VkDescriptorImageInfo{ VK_NULL_HANDLE, m_mipViews[i], VK_IMAGE_LAYOUT_GENERAL })
				.writeImage(1, VkDescriptorImageInfo{ m_sampler, m_mipViews[i + 1], VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL })
				.build(m_upsampleSets[i]->descriptorSet(0));

			DescriptorWriter{ *m_downsampleSetLayout }
				.writeImage(0, VkDescriptorImageInfo{ VK_NULL_HANDLE, m_mipViews[i + 1], VK_IMAGE_LAYOUT_GENERAL })
				.writeImage(1, VkDescriptorImageInfo{ m_sampler, m_mipViews[i], VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL })
				.build(m_downsampleSets[i]->descriptorSet(0));
		}
	}

	void BloomPass::execute(FrameGraphResourcePool& resources, const FrameInfo& frameInfo)
	{
		VkCommandBuffer cmd = frameInfo.commandBuffer;
		auto& bloom = resources.texture(m_bloom);

		extractBrightRegions(cmd, frameInfo);
		downSample(cmd, bloom);
		upSample(cmd, bloom);
	}

	void BloomPass::drawUI()
	{
		ImGui::SliderFloat("Threshold", &m_threshold.threshold, 0.0f, 2.0f);
		ImGui::SliderFloat("Soft Threshold", &m_threshold.softThreshold, 0.0f, 2.0f);
		ImGui::SliderFloat("Downsample Scale", &m_downsample.filterScale, 0.0f, 2.0f);
		ImGui::SliderFloat("Upsample Scale", &m_upsample.filterScale, 0.0f, 2.0f);
	}

	void BloomPass::extractBrightRegions(VkCommandBuffer cmd, const FrameInfo& frameInfo)
	{
		m_thresholdPipeline->bind(cmd);
		m_thresholdPipeline->bindDescriptorSet(cmd, 0, m_thresholdSet->descriptorSet(0));
		m_thresholdPipeline->pushConstants(cmd, VK_SHADER_STAGE_COMPUTE_BIT, m_threshold);

		Tools::vk::cmdDispatch(cmd, frameInfo.swapChainExtent, { 16, 16 });
	}

	void BloomPass::downSample(VkCommandBuffer cmd, Texture& bloom)
	{
		AGX_ASSERT(bloom.image().layout() == VK_IMAGE_LAYOUT_GENERAL);

		m_downsamplePipeline->bind(cmd);

		VkImageMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.image = bloom.image();
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;
		barrier.subresourceRange.levelCount = 1;

		for (uint32_t i = 1; i < BLOOM_MIP_LEVELS; i++)
		{
			uint32_t srcMip = i - 1;
			uint32_t dstMip = i;

			barrier.subresourceRange.baseMipLevel = srcMip;
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

			m_downsample.mipLevel = srcMip;
			
			m_downsamplePipeline->bindDescriptorSet(cmd, 0, m_downsampleSets[srcMip]->descriptorSet(0));
			m_downsamplePipeline->pushConstants(cmd, VK_SHADER_STAGE_COMPUTE_BIT, m_downsample);

			VkExtent2D mipExtent = { bloom.image().width() >> dstMip, bloom.image().height() >> dstMip};
			Tools::vk::cmdDispatch(cmd, mipExtent, { 16, 16 });
		}

		barrier.subresourceRange.baseMipLevel = BLOOM_MIP_LEVELS - 1;
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
	}

	void BloomPass::upSample(VkCommandBuffer cmd, Texture& bloom)
	{
		m_upsamplePipeline->bind(cmd);

		VkImageMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.image = bloom.image();
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;
		barrier.subresourceRange.levelCount = 1;

		for (uint32_t i = BLOOM_MIP_LEVELS - 1; i > 0; i--)
		{
			uint32_t srcMip = i;
			uint32_t dstMip = i - 1;

			// Transition the dst mip level to general (write)
			barrier.subresourceRange.baseMipLevel = dstMip;
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

			// Upsample the mip level
			m_upsamplePipeline->bindDescriptorSet(cmd, 0, m_upsampleSets[dstMip]->descriptorSet(0));
			m_upsamplePipeline->pushConstants(cmd, VK_SHADER_STAGE_COMPUTE_BIT, m_upsample);

			VkExtent2D mipExtent = { bloom.image().width() >> dstMip, bloom.image().height() >> dstMip };
			Tools::vk::cmdDispatch(cmd, mipExtent, { 16, 16 });

			// Transition the dst mip level back to shader read 
			barrier.subresourceRange.baseMipLevel = dstMip;
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
		}

		// Transition all mip levels back to general
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = BLOOM_MIP_LEVELS;
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
}