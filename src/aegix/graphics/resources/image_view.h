#pragma once

#include "graphics/vulkan/volk_include.h"

namespace Aegix::Graphics
{
	class Image;

	class ImageView
	{
	public:
		struct CreateInfo
		{
			static constexpr uint32_t USE_IMAGE_MIP_LEVELS = 0;
			static constexpr uint32_t USE_IMAGE_LAYERS = 0;

			uint32_t baseMipLevel = 0;
			uint32_t levelCount = USE_IMAGE_MIP_LEVELS;
			uint32_t baseLayer = 0;
			uint32_t layerCount = USE_IMAGE_LAYERS;
			VkImageViewType viewType = VK_IMAGE_VIEW_TYPE_2D;
		};

		ImageView() = default;
		explicit ImageView(const CreateInfo& info, const Image& image);
		ImageView(const ImageView&) = delete;
		ImageView(ImageView&& other) noexcept;
		~ImageView();

		auto operator=(const ImageView&) -> ImageView& = delete;
		auto operator=(ImageView&& other) noexcept -> ImageView&;

		operator VkImageView() const { return m_imageView; }

		[[nodiscard]] auto imageView() const -> VkImageView { return m_imageView; }

		//void create(const Image& image, const CreateInfo& config);
		//void create(const Image& image, uint32_t baseMipLevel, uint32_t levelCount, uint32_t baseLayer, uint32_t layerCount);
		//void create2D(const Image& image, uint32_t baseMipLevel = 0, uint32_t levelCount = 1);
		//void createCube(const Image& image, uint32_t baseMipLevel = 0, uint32_t levelCount = 1);

	private:
		void destroy();

		VkImageView m_imageView = VK_NULL_HANDLE;
	};
}
