#include "pch.h"
#include "image_view.h"

#include "graphics/resources/image.h"
#include "graphics/vulkan/vulkan_context.h"
#include "graphics/vulkan/vulkan_tools.h"

namespace Aegis::Graphics
{
	ImageView::ImageView(const CreateInfo& info, const Image& image)
	{
		AGX_ASSERT_X(info.baseMipLevel + info.levelCount <= image.mipLevels(), "Invalid mip level range");
		AGX_ASSERT_X(info.baseLayer + info.layerCount <= image.layerCount(), "Invalid layer range");

		VkImageViewCreateInfo viewInfo{
			.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			.image = image.image(),
			.viewType = info.viewType,
			.format = image.format(),
			.subresourceRange = VkImageSubresourceRange{
				.aspectMask = VulkanContext::device().findAspectFlags(image.format()),
				.baseMipLevel = info.baseMipLevel,
				.levelCount = info.levelCount == CreateInfo::USE_IMAGE_MIP_LEVELS ? image.mipLevels() : info.levelCount,
				.baseArrayLayer = info.baseLayer,
				.layerCount = info.layerCount == CreateInfo::USE_IMAGE_LAYERS ? image.layerCount() : info.layerCount
			}
		};
		VK_CHECK(vkCreateImageView(VulkanContext::device(), &viewInfo, nullptr, &m_imageView));
	}

	ImageView::ImageView(ImageView&& other) noexcept
		: m_imageView{ other.m_imageView }
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

	void ImageView::destroy()
	{
		VulkanContext::destroy(m_imageView);
		m_imageView = VK_NULL_HANDLE;
	}
}
