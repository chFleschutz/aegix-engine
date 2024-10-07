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
			VkFormat format = VK_FORMAT_R8G8B8A8_SRGB;
		};

		Texture(VulkanDevice& device, const std::filesystem::path& texturePath, const Texture::Config& config);
		Texture(VulkanDevice& device, const glm::vec4& color, uint32_t width, uint32_t height, const Texture::Config& config);
		Texture(const Texture&) = delete;
		Texture(Texture&&) = delete;
		~Texture();

		Texture& operator=(const Texture&) = delete;
		Texture& operator=(Texture&&) = delete;

		VkFormat format() const { return m_format; }
		VkImage image() const { return m_image; }
		VkImageView imageView() const { return m_imageView; }

	private:
		void loadTexture(const std::filesystem::path& filePath);
		void createImage(uint32_t width, uint32_t height, const Buffer& buffer);
		void createImageView();

		VulkanDevice& m_device;

		VkFormat m_format;
		VkImage m_image;
		VkDeviceMemory m_imageMemory;
		VkImageView m_imageView;
	};

	class Sampler
	{
	public:
		struct Config
		{
			VkFilter magFilter = VK_FILTER_LINEAR;
			VkFilter minFilter = VK_FILTER_LINEAR;
			VkSamplerAddressMode addressMode = VK_SAMPLER_ADDRESS_MODE_REPEAT;
			bool anisotropy = true;
		};

		explicit Sampler(VulkanDevice& device, const Config& config = {});
		Sampler(const Sampler&) = delete;
		Sampler(Sampler&&) = delete;
		~Sampler();

		VkSampler sampler() const { return m_sampler; }

	private:
		VulkanDevice& m_device;

		VkSampler m_sampler;
	};
}
