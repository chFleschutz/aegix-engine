#include "texture.h"

#include "graphics/vulkan_tools.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <stdexcept>

namespace Aegix::Graphics
{
	Texture::Texture(VulkanDevice& device, const std::filesystem::path& texturePath, VkFormat format)
		: m_device{ device }, m_format{ format }
	{
		createImage(texturePath);
		createImageView();
	}

	Texture::Texture(VulkanDevice& device, uint32_t width, uint32_t height, const glm::vec4& color, VkFormat format)
		: m_device{ device }, m_format{ format }
	{
		createImage(width, height, color);
		createImageView();
	}

	Texture::Texture(VulkanDevice& device, uint32_t width, uint32_t height, VkFormat format, VkImageUsageFlags usage)
		: m_device{ device }, m_format{ format }
	{
		createImage(width, height, usage);
		createImageView();
	}

	Texture::Texture(Texture&& other) noexcept
		: m_device{ other.m_device }, m_format{ other.m_format }, m_extent{ other.m_extent }, m_layout{ other.m_layout },
		m_image{ other.m_image }, m_imageMemory{ other.m_imageMemory }, m_imageView{ other.m_imageView }
	{
		other.m_image = VK_NULL_HANDLE;
		other.m_imageMemory = VK_NULL_HANDLE;
		other.m_imageView = VK_NULL_HANDLE;
	}

	Texture::~Texture()
	{
		destroy();
	}

	Texture& Texture::operator=(Texture&& other) noexcept
	{
		if (this != &other)
		{
			destroy();

			m_format = other.m_format;
			m_extent = other.m_extent;
			m_layout = other.m_layout;
			m_image = other.m_image;
			m_imageMemory = other.m_imageMemory;
			m_imageView = other.m_imageView;

			other.m_image = VK_NULL_HANDLE;
			other.m_imageMemory = VK_NULL_HANDLE;
			other.m_imageView = VK_NULL_HANDLE;
		}
		return *this;
	}

	auto Texture::imageMemoryBarrier(VkImageLayout newLayout) -> VkImageMemoryBarrier
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
		barrier.subresourceRange.levelCount = 1;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;
		barrier.srcAccessMask = Tools::srcAccessMask(barrier.oldLayout);
		barrier.dstAccessMask = Tools::dstAccessMask(barrier.newLayout);

		m_layout = newLayout;

		return barrier;
	}

	void Texture::transitionLayout(VkImageLayout newLayout)
	{
		m_device.transitionImageLayout(m_image, m_format, m_layout, newLayout);
		m_layout = newLayout;
	}

	void Texture::transitionLayout(VkCommandBuffer commandBuffer, VkImageLayout newLayout)
	{
		m_device.transitionImageLayout(commandBuffer, m_image, m_layout, newLayout);
		m_layout = newLayout;
	}

	void Texture::resize(uint32_t width, uint32_t height, VkImageUsageFlags usage)
	{
		destroy();
		createImage(width, height, usage);
		createImageView();
	}

	void Texture::createImage(uint32_t width, uint32_t height, VkImageUsageFlags usage)
	{
		VkImageCreateInfo imageInfo{};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.extent.width = width;
		imageInfo.extent.height = height;
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = 1;
		imageInfo.arrayLayers = 1;
		imageInfo.format = m_format;
		imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageInfo.usage = usage;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageInfo.flags = 0;

		m_extent = { width, height };
		m_layout = imageInfo.initialLayout;

		m_device.createImage(imageInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_image, m_imageMemory);
	}

	void Texture::createImage(uint32_t width, uint32_t height, const Buffer& buffer)
	{
		createImage(width, height, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);

		m_device.transitionImageLayout(m_image, m_format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		m_device.copyBufferToImage(buffer.buffer(), m_image, width, height, 1);
		m_device.transitionImageLayout(m_image, m_format, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	}

	void Texture::createImage(uint32_t width, uint32_t height, const glm::vec4& color)
	{
		auto pixelCount = width * height;

		uint8_t r = static_cast<uint8_t>(color.r * 255);
		uint8_t g = static_cast<uint8_t>(color.g * 255);
		uint8_t b = static_cast<uint8_t>(color.b * 255);
		uint8_t a = static_cast<uint8_t>(color.a * 255);
		uint32_t colorInt = (a << 24) | (b << 16) | (g << 8) | r;
		std::vector<uint32_t> pixels(pixelCount, colorInt);

		Buffer stagingBuffer{ m_device, sizeof(uint32_t), pixelCount, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT };

		stagingBuffer.map();
		stagingBuffer.writeToBuffer(pixels.data());
		stagingBuffer.unmap();

		createImage(width, height, stagingBuffer);
	}

	void Texture::createImage(const std::filesystem::path& filePath)
	{
		int texWidth, texHeight, texChannels;
		stbi_uc* pixels = stbi_load(filePath.string().c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
		VkDeviceSize imageSize = 4 * static_cast<VkDeviceSize>(texWidth) * static_cast<VkDeviceSize>(texHeight);

		if (!pixels)
			throw std::runtime_error("Failed to load texture image: " + filePath.string());

		Buffer stagingBuffer{ m_device, imageSize, 1, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT };

		stagingBuffer.map();
		stagingBuffer.writeToBuffer(pixels);
		stagingBuffer.unmap();

		stbi_image_free(pixels);

		createImage(static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight), stagingBuffer);
	}

	void Texture::createImageView()
	{
		VkImageViewCreateInfo viewInfo{};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = m_image;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = m_format;
		viewInfo.subresourceRange.aspectMask = m_device.findAspectFlags(m_format);
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;

		if (vkCreateImageView(m_device.device(), &viewInfo, nullptr, &m_imageView) != VK_SUCCESS)
			throw std::runtime_error("failed to create texture image view");
	}

	void Texture::destroy()
	{
		VkDevice device = m_device.device();
		if (m_imageView)
			m_device.scheduleDeletion([d = m_device.device(), view = m_imageView]() { vkDestroyImageView(d, view, nullptr); });
		if (m_image)
			m_device.scheduleDeletion([d = m_device.device(), image = m_image]() { vkDestroyImage(d, image, nullptr); });
		if (m_imageMemory)
			m_device.scheduleDeletion([d = m_device.device(), memory = m_imageMemory]() { vkFreeMemory(d, memory, nullptr); });

		m_imageView = VK_NULL_HANDLE;
		m_image = VK_NULL_HANDLE;
		m_imageMemory = VK_NULL_HANDLE;
	}


	Sampler::Sampler(VulkanDevice& device, const Config& config)
		: m_device{ device }
	{
		VkSamplerCreateInfo samplerInfo{};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = config.magFilter;
		samplerInfo.minFilter = config.minFilter;
		samplerInfo.addressModeU = config.addressMode;
		samplerInfo.addressModeV = config.addressMode;
		samplerInfo.addressModeW = config.addressMode;
		samplerInfo.anisotropyEnable = config.anisotropy;
		samplerInfo.maxAnisotropy = m_device.properties().limits.maxSamplerAnisotropy;
		samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		samplerInfo.unnormalizedCoordinates = VK_FALSE;
		samplerInfo.compareEnable = VK_FALSE;
		samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerInfo.minLod = 0.0f;
		samplerInfo.maxLod = 0.0f;
		samplerInfo.mipLodBias = 0.0f;

		if (vkCreateSampler(m_device.device(), &samplerInfo, nullptr, &m_sampler) != VK_SUCCESS)
			throw std::runtime_error("failed to create texture sampler");
	}

	Sampler::~Sampler()
	{
		vkDestroySampler(m_device.device(), m_sampler, nullptr);
	}
}
