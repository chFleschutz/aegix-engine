#include "pch.h"

#include "image_view.h"

#include "graphics/resources/image.h"
#include "graphics/vulkan_tools.h"

namespace Aegix::Graphics
{
	ImageView::ImageView(VulkanDevice& device)
		: m_device{ device }
	{
	}

	ImageView::ImageView(ImageView&& other) noexcept
		: m_device{ other.m_device }, m_imageView{ other.m_imageView }
	{
		other.m_imageView = VK_NULL_HANDLE;
	}

	ImageView::~ImageView()
	{
		destroy();
	}

	ImageView& ImageView::operator=(ImageView&& other) noexcept
	{
		if (this != &other)
		{
			destroy();
			m_imageView = other.m_imageView;
			other.m_imageView = VK_NULL_HANDLE;
		}
		return *this;
	}

	void ImageView::create(const Image& image, const Config& config)
	{
		assert(config.baseMipLevel + config.levelCount <= image.mipLevels() && "Invalid mip level range");
		assert(config.baseLayer + config.layerCount <= image.layerCount() && "Invalid layer range");
		
		destroy();

		VkImageViewCreateInfo viewInfo{};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = image.image();
		viewInfo.viewType = config.viewType;
		viewInfo.format = image.format();
		viewInfo.subresourceRange.aspectMask = m_device.findAspectFlags(image.format());
		viewInfo.subresourceRange.baseMipLevel = config.baseMipLevel;
		viewInfo.subresourceRange.levelCount = config.levelCount;
		viewInfo.subresourceRange.baseArrayLayer = config.baseLayer;
		viewInfo.subresourceRange.layerCount = config.layerCount;

		VK_CHECK(vkCreateImageView(m_device.device(), &viewInfo, nullptr, &m_imageView));
	}

	void ImageView::create(const Image& image, uint32_t baseMipLevel, uint32_t levelCount, uint32_t baseLayer, uint32_t layerCount)
	{
		Config config{
			.baseMipLevel = baseMipLevel,
			.levelCount = levelCount,
			.baseLayer = baseLayer,
			.layerCount = layerCount
		};
		create(image, config);
	}

	void ImageView::create2D(const Image& image, uint32_t baseMipLevel, uint32_t levelCount)
	{
		Config config{
			.baseMipLevel = baseMipLevel,
			.levelCount = levelCount,
			.viewType = VK_IMAGE_VIEW_TYPE_2D
		};
		create(image, config);
	}

	void ImageView::createCube(const Image& image, uint32_t baseMipLevel, uint32_t levelCount)
	{
		Config config{
			.baseMipLevel = baseMipLevel,
			.levelCount = levelCount,
			.baseLayer = 0,
			.layerCount = 6,
			.viewType = VK_IMAGE_VIEW_TYPE_CUBE
		};
		create(image, config);
	}

	void ImageView::destroy()
	{
		m_device.destroyImageView(m_imageView);
		m_imageView = VK_NULL_HANDLE;
	}
}
