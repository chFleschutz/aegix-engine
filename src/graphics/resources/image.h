#pragma once

#include "graphics/buffer.h"
#include "graphics/device.h"

#include <vulkan/vulkan.h>

#include <glm/glm.hpp>

#include <filesystem>

namespace Aegix::Graphics
{
	class Image
	{
	public:
		struct Config
		{
			static constexpr uint32_t CALCULATE_MIP_LEVELS = 0;

			VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;
			VkExtent3D extent = { 1, 1, 1 };
			uint32_t mipLevels = 1;
			uint32_t layerCount = 1;
			VkImageUsageFlags usage = VK_IMAGE_USAGE_SAMPLED_BIT;
			VkImageType imageType = VK_IMAGE_TYPE_2D;
		};

		Image(VulkanDevice& device);
		Image(const Image&) = delete;
		Image(Image&& other) noexcept;
		~Image();

		auto operator=(const Image&) -> Image& = delete;
		auto operator=(Image&& other) noexcept -> Image&;

		[[nodiscard]] auto image() const -> VkImage { return m_image; }
		[[nodiscard]] auto format() const -> VkFormat { return m_format; }
		[[nodiscard]] auto extent() const -> VkExtent3D { return m_extent; }
		[[nodiscard]] auto mipLevels() const -> uint32_t { return m_mipLevels; }
		[[nodiscard]] auto layerCount() const -> uint32_t { return m_layerCount; }
		[[nodiscard]] auto layout() const -> VkImageLayout { return m_layout; }

		void create(const Config& config);
		void create(VkExtent3D extent, VkFormat format, VkImageUsageFlags usage, uint32_t mipLevels = 1, uint32_t layerCount = 1);
		void create(const std::filesystem::path& path, VkFormat format);

		void fill(const glm::vec4& color);
		void fill(const void* data, VkDeviceSize size);
		void fill(const Buffer& buffer);

		void resize(VkExtent3D extent, VkImageUsageFlags usage);

		void transitionLayout(VkCommandBuffer cmd, VkImageLayout newLayout);

		void generateMipmaps(VkCommandBuffer cmd, VkImageLayout finalLayout);

	private:
		void destroy();

		VulkanDevice& m_device;

		VkImage m_image = VK_NULL_HANDLE;
		VkDeviceMemory m_memory = VK_NULL_HANDLE;

		VkFormat m_format = VK_FORMAT_UNDEFINED;
		VkExtent3D m_extent = { 1, 1, 1 };
		uint32_t m_mipLevels = 1;
		uint32_t m_layerCount = 1;
		VkImageLayout m_layout = VK_IMAGE_LAYOUT_UNDEFINED;
	};
}
