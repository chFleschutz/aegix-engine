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

	private:
		void loadTexture(const std::string& filePath);

		VreDevice& mVreDevice;

		VkImage mTextureImage;
		VkDeviceMemory mTextureImageMemory;
	};

} // namespace vre