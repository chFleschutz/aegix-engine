#pragma once

#include "vre_device.h"

#include <vulkan/vulkan.h>

#include <string>

namespace vre
{
	class VreTexture
	{
	public:
		struct CreateInfo
		{
			std::string textureFilePath;
		};

		VreTexture(VreDevice& device, const VreTexture::CreateInfo& createInfo);
		~VreTexture();

		VreTexture(const VreTexture&) = delete;
		VreTexture& operator=(const VreTexture&) = delete;

		VkDescriptorImageInfo descriptorImageInfo();

	private:
		void loadTexture(const std::string& filePath);
		void createImageView();
		void createTextureSampler();

		VreDevice& mVreDevice;

		VkImage mTextureImage;
		VkDeviceMemory mTextureImageMemory;
		VkImageView mTextureImageView;
		VkSampler mTextureSampler;
	};

} // namespace vre