#include "pch.h"
#include "image.h"

#include "graphics/vulkan/vulkan_context.h"
#include "graphics/vulkan/vulkan_tools.h"

namespace Aegix::Graphics
{
	Image::Image(const CreateInfo& info) :
		m_extent{ info.extent },
		m_format{ info.format },
		m_mipLevels{ info.mipLevels },
		m_layerCount{ info.layerCount }
	{
		create(info);
	}

	Image::Image(Image&& other) noexcept
	{
		m_image = other.m_image;
		m_allocation = other.m_allocation;
		m_format = other.m_format;
		m_extent = other.m_extent;
		m_mipLevels = other.m_mipLevels;
		m_layerCount = other.m_layerCount;
		m_layout = other.m_layout;

		other.m_image = VK_NULL_HANDLE;
		other.m_allocation = VK_NULL_HANDLE;
	}

	Image::~Image()
	{
		destroy();
	}

	auto Image::operator=(Image&& other) noexcept -> Image&
	{
		if (this != &other)
		{
			destroy();

			m_image = other.m_image;
			m_allocation = other.m_allocation;
			m_format = other.m_format;
			m_extent = other.m_extent;
			m_mipLevels = other.m_mipLevels;
			m_layerCount = other.m_layerCount;
			m_layout = other.m_layout;
			other.m_image = VK_NULL_HANDLE;
			other.m_allocation = VK_NULL_HANDLE;
		}
		return *this;
	}

	void Image::upload(const Buffer& buffer)
	{
		VkCommandBuffer cmd = VulkanContext::device().beginSingleTimeCommands();
		{
			Tools::vk::cmdTransitionImageLayout(cmd, m_image, m_format, m_layout, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, m_mipLevels, m_layerCount);
			Tools::vk::cmdCopyBufferToImage(cmd, buffer, m_image, m_extent, m_layerCount);
			generateMipmaps(cmd, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		}
		VulkanContext::device().endSingleTimeCommands(cmd);
	}

	void Image::upload(const void* data, VkDeviceSize size)
	{
		Buffer stagingBuffer{ Buffer::stagingBuffer(size) };
		stagingBuffer.singleWrite(data);
		upload(stagingBuffer);
	}

	void Image::create(const CreateInfo& config)
	{
		if (config.mipLevels == CreateInfo::CALCULATE_MIP_LEVELS)
		{
			uint32_t maxDim = std::max(config.extent.width, std::max(config.extent.height, config.extent.depth));
			m_mipLevels = static_cast<uint32_t>(std::floor(std::log2(maxDim))) + 1;
		}

		VkImageCreateInfo imageInfo{
			.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
			.flags = config.flags,
			.imageType = config.imageType,
			.format = m_format,
			.extent = m_extent,
			.mipLevels = m_mipLevels,
			.arrayLayers = m_layerCount,
			.samples = VK_SAMPLE_COUNT_1_BIT,
			.tiling = VK_IMAGE_TILING_OPTIMAL,
			.usage = config.usage,
			.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
			.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
		};

		if (m_mipLevels > 1)
		{
			imageInfo.usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		}

		VmaAllocationCreateInfo allocInfo{
			.usage = VMA_MEMORY_USAGE_AUTO,
		};

		VulkanContext::device().createImage(m_image, m_allocation, imageInfo, allocInfo);
	}

	void Image::copyFrom(VkCommandBuffer cmd, const Buffer& src)
	{
		Tools::vk::cmdTransitionImageLayout(cmd, m_image, m_format, m_layout, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, m_mipLevels, m_layerCount);
		Tools::vk::cmdCopyBufferToImage(cmd, src.buffer(), m_image, m_extent, m_layerCount);
		Tools::vk::cmdTransitionImageLayout(cmd, m_image, m_format, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, m_mipLevels, m_layerCount);
	}

	void Image::resize(VkExtent3D extent, VkImageUsageFlags usage)
	{
		destroy();

		CreateInfo config{
			.format = m_format,
			.extent = extent,
			.mipLevels = m_mipLevels,
			.layerCount = m_layerCount,
			.usage = usage
		};
		create(config);
	}

	void Image::transitionLayout(VkImageLayout newLayout)
	{
		if (m_layout == newLayout)
			return;
		VkCommandBuffer cmd = VulkanContext::device().beginSingleTimeCommands();
		transitionLayout(cmd, newLayout);
		VulkanContext::device().endSingleTimeCommands(cmd);
	}

	void Image::transitionLayout(VkCommandBuffer cmd, VkImageLayout newLayout)
	{
		if (m_layout == newLayout)
			return;

		Tools::vk::cmdTransitionImageLayout(cmd, m_image, m_format, m_layout, newLayout, m_mipLevels, m_layerCount);
		m_layout = newLayout;
	}

	auto Image::transitionLayoutDeferred(VkImageLayout newLayout) -> VkImageMemoryBarrier
	{
		VkImageMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.oldLayout = m_layout;
		barrier.newLayout = newLayout;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = m_image;
		barrier.subresourceRange.aspectMask = Tools::aspectFlags(m_format);
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = m_mipLevels;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = m_layerCount;
		barrier.srcAccessMask = Tools::srcAccessMask(barrier.oldLayout);
		barrier.dstAccessMask = Tools::dstAccessMask(barrier.newLayout);

		m_layout = newLayout;
		return barrier;
	}

	void Image::generateMipmaps(VkCommandBuffer cmd, VkImageLayout finalLayout)
	{
		transitionLayout(cmd, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

		VkImageMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.image = m_image;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = m_layerCount;
		barrier.subresourceRange.levelCount = 1;

		for (uint32_t i = 1; i < m_mipLevels; i++)
		{
			barrier.subresourceRange.baseMipLevel = i - 1;
			barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

			vkCmdPipelineBarrier(cmd,
				VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
				0, nullptr,
				0, nullptr,
				1, &barrier
			);

			VkImageBlit blit{};
			blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			blit.srcSubresource.mipLevel = i - 1;
			blit.srcSubresource.baseArrayLayer = 0;
			blit.srcSubresource.layerCount = m_layerCount;
			blit.srcOffsets[0] = { 0, 0, 0 };
			blit.srcOffsets[1].x = static_cast<int32_t>(m_extent.width >> (i - 1));
			blit.srcOffsets[1].y = static_cast<int32_t>(m_extent.height >> (i - 1));
			blit.srcOffsets[1].z = 1;
			blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			blit.dstSubresource.mipLevel = i;
			blit.srcSubresource.baseArrayLayer = 0;
			blit.dstSubresource.layerCount = m_layerCount;
			blit.dstOffsets[0] = { 0, 0, 0 };
			blit.dstOffsets[1].x = static_cast<int32_t>(m_extent.width >> i);
			blit.dstOffsets[1].y = static_cast<int32_t>(m_extent.height >> i);
			blit.dstOffsets[1].z = 1;

			vkCmdBlitImage(cmd,
				m_image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				m_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				1, &blit,
				VK_FILTER_LINEAR);

			barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
			barrier.newLayout = finalLayout;
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			barrier.dstAccessMask = Tools::dstAccessMask(finalLayout);

			vkCmdPipelineBarrier(cmd,
				VK_PIPELINE_STAGE_TRANSFER_BIT, Tools::dstStage(barrier.dstAccessMask), 0,
				0, nullptr,
				0, nullptr,
				1, &barrier);
		}

		barrier.subresourceRange.baseMipLevel = m_mipLevels - 1;
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.newLayout = finalLayout;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = Tools::dstAccessMask(finalLayout);

		vkCmdPipelineBarrier(cmd,
			VK_PIPELINE_STAGE_TRANSFER_BIT, Tools::dstStage(barrier.dstAccessMask), 0,
			0, nullptr,
			0, nullptr,
			1, &barrier);

		m_layout = finalLayout;
	}

	void Image::destroy()
	{
		VulkanContext::destroy(m_image, m_allocation);
		m_image = VK_NULL_HANDLE;
		m_allocation = VK_NULL_HANDLE;
	}
}
