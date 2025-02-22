#pragma once

#include "graphics/device.h"

#include <memory>
#include <vector>

namespace Aegix::Graphics
{
	class SwapChain
	{
	public:
		SwapChain(VulkanDevice& device, VkExtent2D windowExtent);
		SwapChain(VulkanDevice& device, VkExtent2D windowExtent, std::shared_ptr<SwapChain> previous);
		~SwapChain();

		SwapChain(const SwapChain&) = delete;
		void operator=(const SwapChain&) = delete;

		VkImageView colorImageView(int index) const { return mColorImageViews[index]; }
		VkImage colorImage(int index) const { return mColorImages[index]; }

		uint32_t width() const { return mSwapChainExtent.width; }
		uint32_t height() const { return mSwapChainExtent.height; }

		float extentAspectRatio() const { return static_cast<float>(mSwapChainExtent.width) / static_cast<float>(mSwapChainExtent.height); }
		VkExtent2D extend() const { return mSwapChainExtent; }
		size_t imageCount() const { return mColorImages.size(); }
		VkFormat swapChainImageFormat() const { return mSwapChainImageFormat; }
		VkFormat findDepthFormat();

		VkResult acquireNextImage(uint32_t* imageIndex);
		VkResult submitCommandBuffers(const VkCommandBuffer* buffers, uint32_t* imageIndex);

		bool compareSwapFormats(const SwapChain& swapchain) const 
		{
			return swapchain.mSwapChainImageFormat == mSwapChainImageFormat;
		}

		void transitionColorAttachment(VkCommandBuffer commandBuffer, uint32_t imageIndex);
		void transitionPresent(VkCommandBuffer commandBuffer, uint32_t imageIndex);

	private:
		void init();
		void createSwapChain();
		void createImageViews();
		void createSyncObjects();

		VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) const;
		VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) const;
		VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) const;
		uint32_t chooseImageCount(const VkSurfaceCapabilitiesKHR& capabilities) const;

		VkFormat mSwapChainImageFormat;
		VkExtent2D mSwapChainExtent;

		std::vector<VkImage> mColorImages;
		std::vector<VkImageView> mColorImageViews;

		VulkanDevice& m_device;
		VkExtent2D m_windowExtent;

		VkSwapchainKHR m_swapChain;
		std::shared_ptr<SwapChain> m_oldSwapChain;

		std::vector<VkSemaphore> m_imageAvailableSemaphores;
		std::vector<VkSemaphore> m_renderFinishedSemaphores;
		std::vector<VkFence> m_inFlightFences;
		std::vector<VkFence> m_imagesInFlight;
		size_t m_currentFrame = 0;
	};
}
