#include "texture.h"

#include "graphics/vulkan_tools.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <cassert>

namespace Aegix::Graphics
{
	Texture::Texture(VulkanDevice& device, const Config& config)
		: m_device{ device }
	{
		createImage(config);
		createImageView(config);
		createSampler(config);
	}

	Texture::Texture(VulkanDevice& device, const std::filesystem::path& texturePath, VkFormat format)
		: m_device{ device }
	{
		int texWidth, texHeight, texChannels;
		stbi_uc* pixels = stbi_load(texturePath.string().c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
		VkDeviceSize imageSize = 4 * static_cast<VkDeviceSize>(texWidth) * static_cast<VkDeviceSize>(texHeight);

		assert(pixels && "Failed to load texture image");

		Buffer stagingBuffer{ m_device, imageSize, 1, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT };

		stagingBuffer.map();
		stagingBuffer.writeToBuffer(pixels);
		stagingBuffer.unmap();

		stbi_image_free(pixels);

		Config config{};
		config.extent = { static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight) };
		config.format = format;
		config.usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;

		createImage(config);
		createImageView(config);
		createSampler(config);

		fill(stagingBuffer);
	}

	Texture::Texture(VulkanDevice& device, uint32_t width, uint32_t height, const glm::vec4& color, VkFormat format)
		: m_device{ device }
	{
		Config config{};
		config.extent = { width, height };
		config.format = format;
		config.usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;

		createImage(config);
		createImageView(config);
		createSampler(config);

		fill(color);
	}

	Texture::Texture(VulkanDevice& device, uint32_t width, uint32_t height, VkFormat format, VkImageUsageFlags usage)
		: m_device{ device }
	{
		Config config{};
		config.extent = { width, height };
		config.format = format;
		config.usage = usage;

		createImage(config);
		createImageView(config);
		createSampler(config);
	}


	Texture::Texture(VulkanDevice& device, const SwapChain& swapChain)
		: m_device{ device }, m_format{ swapChain.format() }, m_extent{ swapChain.extend() }, m_layout{ VK_IMAGE_LAYOUT_UNDEFINED },
		m_image{ swapChain.image(0) }, m_imageView{ swapChain.imageView(0) }
	{
		Config config{};
		createSampler(config);
	}

	Texture::Texture(Texture&& other) noexcept
		: m_device{ other.m_device }, m_format{ other.m_format }, m_extent{ other.m_extent }, m_layout{ other.m_layout },
		m_image{ other.m_image }, m_imageMemory{ other.m_imageMemory }, m_imageView{ other.m_imageView }, 
		m_sampler{ other.m_sampler }
	{
		other.m_image = VK_NULL_HANDLE;
		other.m_imageMemory = VK_NULL_HANDLE;
		other.m_imageView = VK_NULL_HANDLE;
		other.m_sampler = VK_NULL_HANDLE;
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
			m_sampler = other.m_sampler;

			other.m_image = VK_NULL_HANDLE;
			other.m_imageMemory = VK_NULL_HANDLE;
			other.m_imageView = VK_NULL_HANDLE;
			other.m_sampler = VK_NULL_HANDLE;
		}
		return *this;
	}

	auto Texture::descriptorImageInfo() const -> VkDescriptorImageInfo
	{
		VkDescriptorImageInfo info{};
		info.sampler = m_sampler;
		info.imageView = m_imageView;
		info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		return info;
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

	void Texture::fill(const glm::vec4& color)
	{
		auto pixelCount = m_extent.width * m_extent.height;

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

		fill(stagingBuffer);
	}

	void Texture::fill(const Buffer& buffer)
	{
		m_device.transitionImageLayout(m_image, m_format, m_layout, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		m_device.copyBufferToImage(buffer.buffer(), m_image, m_extent.width, m_extent.height, 1);
		m_device.transitionImageLayout(m_image, m_format, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
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
		m_device.scheduleDeletion(m_imageView);
		m_device.scheduleDeletion(m_image);
		m_device.scheduleDeletion(m_imageMemory);

		Config config{};
		config.extent = { width, height };
		config.format = m_format;
		config.usage = usage;

		createImage(config);
		createImageView(config);
	}

	void Texture::update(const SwapChain& swapChain)
	{
		uint32_t index = swapChain.currentImageIndex();
		update(swapChain.image(index), swapChain.imageView(index));
	}

	void Texture::update(VkImage image, VkImageView imageView)
	{
		m_image = image;
		m_imageView = imageView;
	}

	void Texture::createImage(const Config& config)
	{
		VkImageCreateInfo imageInfo{};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.extent.width = config.extent.width;
		imageInfo.extent.height = config.extent.height;
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = 1;
		imageInfo.arrayLayers = 1;
		imageInfo.format = config.format;
		imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageInfo.usage = config.usage;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageInfo.flags = 0;

		m_extent = { imageInfo.extent.width, imageInfo.extent.height };
		m_layout = imageInfo.initialLayout;
		m_format = imageInfo.format;

		m_device.createImage(imageInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_image, m_imageMemory);
	}

	void Texture::createImageView(const Config& config)
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

		VK_CHECK(vkCreateImageView(m_device.device(), &viewInfo, nullptr, &m_imageView))
	}

	void Texture::createSampler(const Config& config)
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
		samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
		samplerInfo.unnormalizedCoordinates = VK_FALSE;
		samplerInfo.compareEnable = VK_FALSE;
		samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerInfo.mipLodBias = 0.0f;
		samplerInfo.minLod = 0.0f;
		samplerInfo.maxLod = 0.0f;

		VK_CHECK(vkCreateSampler(m_device.device(), &samplerInfo, nullptr, &m_sampler))
	}

	void Texture::destroy()
	{
		if (m_imageView)
			m_device.scheduleDeletion(m_imageView);
		if (m_image)
			m_device.scheduleDeletion(m_image);
		if (m_imageMemory)
			m_device.scheduleDeletion(m_imageMemory);
		if (m_sampler)
			m_device.scheduleDeletion(m_sampler);
	}
}
