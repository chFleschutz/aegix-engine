#pragma once

#include "graphics/vulkan/volk_include.h"

namespace Aegix::Graphics
{
	class SwapChain
	{
	public:
		struct ImageSync
		{
			VkFence inFlightFence;
			VkSemaphore presentReady;
		};

		SwapChain(VkExtent2D windowExtent);
		SwapChain(const SwapChain&) = delete;
		~SwapChain();

		void operator=(const SwapChain&) = delete;

		[[nodiscard]] auto extent() const -> VkExtent2D { return m_extent; }
		[[nodiscard]] auto width() const -> uint32_t { return m_extent.width; }
		[[nodiscard]] auto height() const -> uint32_t { return m_extent.height; }
		[[nodiscard]] auto aspectRatio() const -> float { return static_cast<float>(m_extent.width) / static_cast<float>(m_extent.height); }
		[[nodiscard]] auto imageCount() const -> size_t { return m_images.size(); }
		[[nodiscrad]] auto currentImage() const -> VkImage { return m_images[m_imageIndex]; }
		[[nodiscard]] auto findDepthFormat() -> VkFormat;
		[[nodiscard]] auto presentReadySemaphore() const -> VkSemaphore { return m_imageSync[m_imageIndex].presentReady; }

		auto acquireNextImage(VkSemaphore imageAvailable) -> VkResult;
		void waitForImageInFlight(VkFence frameFence);
		auto present() -> VkResult;

		auto compareSwapFormats(const SwapChain& swapchain) const -> bool { return swapchain.m_format == m_format; }

		void resize(VkExtent2D extent);

	private:
		void createSwapChain();
		void createImageViews();
		void destroyImageViews();
		void destroySwapChain(VkSwapchainKHR swapChain);

		auto chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) const -> VkSurfaceFormatKHR;
		auto chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) const -> VkPresentModeKHR;
		auto chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) const -> VkExtent2D;
		auto chooseImageCount(const VkSurfaceCapabilitiesKHR& capabilities) const -> uint32_t;

		VkFormat m_format;
		VkExtent2D m_extent;
		VkExtent2D m_windowExtent;
		VkSwapchainKHR m_swapChain{ VK_NULL_HANDLE };
		std::vector<VkImage> m_images;
		std::vector<VkImageView> m_imageViews;
		std::vector<ImageSync> m_imageSync;
		uint32_t m_imageIndex{ 0 };
	};
}
