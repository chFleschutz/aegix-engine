#pragma once

#include "graphics/device.h"

namespace Aegix::Graphics
{
	class Image;

	class ImageView
	{
	public:
		struct Config
		{
			uint32_t baseMipLevel = 0;
			uint32_t levelCount = 1;
			uint32_t baseLayer = 0;
			uint32_t layerCount = 1;
			VkImageViewType viewType = VK_IMAGE_VIEW_TYPE_2D;
		};

		explicit ImageView(VulkanDevice& device);
		ImageView(const ImageView&) = delete;
		ImageView(ImageView&& other) noexcept;
		~ImageView();

		auto operator=(const ImageView&)->ImageView & = delete;
		auto operator=(ImageView&& other) noexcept -> ImageView&;

		operator VkImageView() const { return m_imageView; }

		[[nodiscard]] auto imageView() const -> VkImageView { return m_imageView; }

		void create(const Image& image, const Config& config);
		void create(const Image& image, uint32_t baseMipLevel, uint32_t levelCount, uint32_t baseLayer, uint32_t layerCount);
		void create2D(const Image& image, uint32_t baseMipLevel = 0, uint32_t levelCount = 1);
		void createCube(const Image& image, uint32_t baseMipLevel = 0, uint32_t levelCount = 1);

	private:
		void destroy();

		VulkanDevice& m_device;
		VkImageView m_imageView = VK_NULL_HANDLE;
	};
}
