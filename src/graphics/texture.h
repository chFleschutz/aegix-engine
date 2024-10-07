#pragma once

#include "graphics/buffer.h"
#include "graphics/device.h"
#include "utils/math_utils.h"

#include <vulkan/vulkan.h>

#include <filesystem>

namespace Aegix::Graphics
{
	class Texture
	{
	public:
		struct Config
		{
			VkFilter magFilter = VK_FILTER_LINEAR;
			VkFilter minFilter = VK_FILTER_LINEAR;
			VkSamplerAddressMode addressMode = VK_SAMPLER_ADDRESS_MODE_REPEAT;
			VkFormat format = VK_FORMAT_R8G8B8A8_SRGB;
		};

		Texture(VulkanDevice& device, const std::filesystem::path& texturePath, const Texture::Config& config);
		Texture(VulkanDevice& device, const glm::vec4& color, uint32_t width, uint32_t height, const Texture::Config& config);
		~Texture();

		Texture(const Texture&) = delete;
		Texture& operator=(const Texture&) = delete;

		VkDescriptorImageInfo descriptorImageInfo() const;
		VkImageView imageView() const { return m_textureImageView; }
		VkFormat format() const { return m_format; }

	private:
		void loadTexture(const std::filesystem::path& filePath);
		void createImage(uint32_t width, uint32_t height, const Buffer& buffer);
		void createImageView();
		void createTextureSampler(VkSamplerAddressMode addressMode, VkFilter magFilter, VkFilter minFilter);

		VulkanDevice& m_device;

		VkFormat m_format;
		VkImage m_textureImage;
		VkDeviceMemory m_textureImageMemory;
		VkImageView m_textureImageView;
		VkSampler m_textureSampler;
	};
}
