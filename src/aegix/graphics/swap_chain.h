#pragma once

#include "graphics/device.h"

namespace Aegix::Graphics
{
	class SwapChain
	{
	public:
		SwapChain(VulkanDevice& device, VkExtent2D windowExtent);
		SwapChain(const SwapChain&) = delete;
		~SwapChain();

		void operator=(const SwapChain&) = delete;

		[[nodiscard]] auto device() const -> VulkanDevice& { return m_device; }

		[[nodiscard]] auto imageCount() const -> size_t { return m_images.size(); }
		[[nodiscard]] auto currentImageIndex() const -> uint32_t { return m_currentImageIndex; }

		[[nodiscard]] auto currentImageView() const -> VkImageView { return m_imageViews[m_currentImageIndex]; }
		[[nodiscrad]] auto currentImage() const -> VkImage { return m_images[m_currentImageIndex]; }
		[[nodiscard]] auto imageView(int index) const -> VkImageView { return m_imageViews[index]; }
		[[nodiscard]] auto image(int index) const -> VkImage { return m_images[index]; }

		[[nodiscard]] auto extent() const -> VkExtent2D { return m_extent; }
		[[nodiscard]] auto width() const -> uint32_t { return m_extent.width; }
		[[nodiscard]] auto height() const -> uint32_t { return m_extent.height; }
		[[nodiscard]] auto aspectRatio() const -> float { return static_cast<float>(m_extent.width) / static_cast<float>(m_extent.height); }

		[[nodiscard]] auto format() const -> VkFormat { return m_format; }
		[[nodiscard]] auto findDepthFormat() -> VkFormat;

		auto acquireNextImage() -> VkResult;
		auto submitCommandBuffers(const VkCommandBuffer* buffers) -> VkResult;

		auto compareSwapFormats(const SwapChain& swapchain) const -> bool { return swapchain.m_format == m_format; }

		void resize(VkExtent2D extent);

	private:
		void createSwapChain();
		void createImageViews();
		void createSyncObjects();
		void destroyImageViews();
		void destroySwapChain(VkSwapchainKHR swapChain);

		auto chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) const -> VkSurfaceFormatKHR;
		auto chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) const -> VkPresentModeKHR;
		auto chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) const -> VkExtent2D;
		auto chooseImageCount(const VkSurfaceCapabilitiesKHR& capabilities) const -> uint32_t;

		VulkanDevice& m_device;

		VkFormat m_format;
		VkExtent2D m_extent;
		VkExtent2D m_windowExtent;

		VkSwapchainKHR m_swapChain = VK_NULL_HANDLE;
		std::vector<VkImage> m_images;
		std::vector<VkImageView> m_imageViews;
		uint32_t m_currentImageIndex = 0;

		std::vector<VkSemaphore> m_imageAvailableSemaphores;
		std::vector<VkSemaphore> m_renderFinishedSemaphores;
		std::vector<VkFence> m_inFlightFences;
		std::vector<VkFence> m_imagesInFlight;
		size_t m_currentFrame = 0;
	};
}
