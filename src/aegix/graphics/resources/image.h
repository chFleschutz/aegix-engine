#pragma once

#include "graphics/resources/buffer.h"

#include <vk_mem_alloc.h>

namespace Aegix::Graphics
{
	class Image
	{
	public:
		struct CreateInfo
		{
			static constexpr uint32_t CALCULATE_MIP_LEVELS = 0;

			VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;
			VkExtent3D extent = { 1, 1, 1 };
			uint32_t mipLevels = CALCULATE_MIP_LEVELS;
			uint32_t layerCount = 1;
			VkImageUsageFlags usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
			VkImageType imageType = VK_IMAGE_TYPE_2D;
			VkImageCreateFlags flags = 0;
		};

		Image() = default;
		explicit Image(const CreateInfo& info);
		Image(const Image&) = delete;
		Image(Image&& other) noexcept;
		~Image();

		auto operator=(const Image&) -> Image& = delete;
		auto operator=(Image&& other) noexcept -> Image&;

		operator VkImage() const { return m_image; }

		[[nodiscard]] auto image() const -> VkImage { return m_image; }
		[[nodiscard]] auto format() const -> VkFormat { return m_format; }
		[[nodiscard]] auto extent() const -> VkExtent3D { return m_extent; }
		[[nodiscard]] auto width() const -> uint32_t { return m_extent.width; }
		[[nodiscard]] auto height() const -> uint32_t { return m_extent.height; }
		[[nodiscard]] auto depth() const -> uint32_t { return m_extent.depth; }
		[[nodiscard]] auto mipLevels() const -> uint32_t { return m_mipLevels; }
		[[nodiscard]] auto layerCount() const -> uint32_t { return m_layerCount; }
		[[nodiscard]] auto layout() const -> VkImageLayout { return m_layout; }

		void upload(const Buffer& buffer);
		void upload(const void* data, VkDeviceSize size);

		//void fill(const Buffer& buffer);
		//void fill(const void* data, VkDeviceSize size);
		//void fillSFLOAT(const glm::vec4& color);
		//void fillRGBA8(const glm::vec4& color);

		void copyFrom(VkCommandBuffer cmd, const Buffer& src);

		void transitionLayout(VkImageLayout newLayout);
		void transitionLayout(VkCommandBuffer cmd, VkImageLayout newLayout);
		auto transitionLayoutDeferred(VkImageLayout newLayout) -> VkImageMemoryBarrier;

		/// @brief Sets the current image layout WITHOUT any doing any transitions
		void setLayout(VkImageLayout layout) { m_layout = layout; }

		void generateMipmaps(VkCommandBuffer cmd, VkImageLayout finalLayout);

	private:
		//void create(VkExtent3D extent, VkFormat format, VkImageUsageFlags usage, uint32_t mipLevels = 1, uint32_t layerCount = 1);
		//void create(const std::filesystem::path& path, VkFormat format);
		void create(const CreateInfo& config);
		void destroy();

		VkImage m_image = VK_NULL_HANDLE;
		VmaAllocation m_allocation = VK_NULL_HANDLE;
		VkExtent3D m_extent = { 1, 1, 1 };
		VkFormat m_format = VK_FORMAT_UNDEFINED;
		uint32_t m_mipLevels = 1;
		uint32_t m_layerCount = 1;
		VkImageLayout m_layout = VK_IMAGE_LAYOUT_UNDEFINED;
	};
}
