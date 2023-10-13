#pragma once

#include "renderer/device.h"

#include <vulkan/vulkan.h>

#include <string>

namespace vre
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

		VulkanDevice& mDevice;

		VkImage mTextureImage;
		VkDeviceMemory mTextureImageMemory;
		VkImageView mTextureImageView;
		VkSampler mTextureSampler;
	};

} // namespace vre