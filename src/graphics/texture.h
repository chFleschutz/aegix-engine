#pragma once

#include "graphics/buffer.h"
#include "graphics/device.h"
#include "utils/math_utils.h"

#include <vulkan/vulkan.h>

#include <filesystem>

namespace Aegix::Graphics
{
	/// @brief Represents a Texture on the gpu and wraps a VkImage, VkDeviceMemory and VkImageView
	class Texture
	{
	public:
		/// @brief Creates a texture from a file, supports: jpeg, png, tga, bmp, psd, gif, hdr, pic, pnm (see: https://github.com/nothings/stb/blob/master/stb_image.h)
		/// @note The texture will be in VK_LAYOUT_SHADER_READ_ONLY_OPTIMAL layout
		Texture(VulkanDevice& device, const std::filesystem::path& texturePath, VkFormat format);

		/// @brief Creates a texture with a single color
		/// @note The texture will be in VK_LAYOUT_SHADER_READ_ONLY_OPTIMAL layout
		Texture(VulkanDevice& device, uint32_t width, uint32_t height, const glm::vec4& color, VkFormat format);

		/// @brief Creates an empty texture (primarily for render targets)
		/// @note The texture will be in VK_LAYOUT_UNDEFINED layout
		Texture(VulkanDevice& device, uint32_t width, uint32_t height, VkFormat format, VkImageUsageFlags usage);

		Texture(const Texture& other) = delete;
		Texture(Texture&& other) noexcept;
		~Texture();

		Texture& operator=(const Texture&) = delete;
		Texture& operator=(Texture&&) = delete;

		VkFormat format() const { return m_format; }
		VkExtent2D extent() const { return m_extent; }
		uint32_t width() const { return m_extent.width; }
		uint32_t height() const { return m_extent.height; }
		VkImage image() const { return m_image; }
		VkImageView imageView() const { return m_imageView; }

		void transitionLayout(VkImageLayout newLayout);
		void transitionLayout(VkCommandBuffer commandBuffer, VkImageLayout newLayout);

	private:
		void createImage(uint32_t width, uint32_t height, VkImageUsageFlags usage);
		void createImage(uint32_t width, uint32_t height, const Buffer& buffer);
		void createImage(uint32_t width, uint32_t height, const glm::vec4& color);
		void createImage(const std::filesystem::path& filePath);
		void createImageView();

		VulkanDevice& m_device;

		VkFormat m_format;
		VkExtent2D m_extent;
		VkImageLayout m_layout = VK_IMAGE_LAYOUT_UNDEFINED;
		VkImage m_image = VK_NULL_HANDLE;
		VkDeviceMemory m_imageMemory = VK_NULL_HANDLE;
		VkImageView m_imageView = VK_NULL_HANDLE;
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
