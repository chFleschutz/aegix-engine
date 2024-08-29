#pragma once

#include "graphics/device.h"

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
		};

		Texture(VulkanDevice& device, const std::filesystem::path& texturePath, const Texture::Config& config);
		~Texture();

		Texture(const Texture&) = delete;
		Texture& operator=(const Texture&) = delete;

		VkDescriptorImageInfo descriptorImageInfo() const;

	private:
		void loadTexture(const std::filesystem::path& filePath);
		void createImageView();
		void createTextureSampler(VkSamplerAddressMode addressMode, VkFilter magFilter, VkFilter minFilter);

		VulkanDevice& m_device;

		VkImage m_textureImage;
		VkDeviceMemory m_textureImageMemory;
		VkImageView m_textureImageView;
		VkSampler m_textureSampler;
	};
}
