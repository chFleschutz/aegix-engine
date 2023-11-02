#pragma once

#include "graphics/device.h"

#include <vulkan/vulkan.h>

#include <string>

namespace VEGraphics
{
	class Texture
	{
	public:
		struct CreateInfo
		{
			std::string textureFilePath;
		};

		Texture(VulkanDevice& device, const Texture::CreateInfo& createInfo);
		~Texture();

		Texture(const Texture&) = delete;
		Texture& operator=(const Texture&) = delete;

		VkDescriptorImageInfo descriptorImageInfo();

	private:
		void loadTexture(const std::string& filePath);
		void createImageView();
		void createTextureSampler();

		VulkanDevice& m_device;

		VkImage m_textureImage;
		VkDeviceMemory m_textureImageMemory;
		VkImageView m_textureImageView;
		VkSampler m_textureSampler;
	};

} // namespace vre