#pragma once

#include "vre_device.h"

#include <vulkan/vulkan.h>

#include <memory>
#include <string>
#include <vector>

namespace vre
{

	class VreSwapChain
	{
	public:
		static constexpr int MAX_FRAMES_IN_FLIGHT = 2;

		VreSwapChain(VreDevice& deviceRef, VkExtent2D windowExtent);
		VreSwapChain(VreDevice& deviceRef, VkExtent2D windowExtent, std::shared_ptr<VreSwapChain> previous);
		~VreSwapChain();

		VreSwapChain(const VreSwapChain&) = delete;
		void operator=(const VreSwapChain&) = delete;

		VkFramebuffer frameBuffer(int index) { return mSwapChainFramebuffers[index]; }
		VkRenderPass renderPass() { return mRenderPass; }
		VkImageView imageView(int index) { return mSwapChainImageViews[index]; }
		size_t imageCount() { return mSwapChainImages.size(); }
		VkFormat swapChainImageFormat() { return mSwapChainImageFormat; }
		VkExtent2D swapChainExtent() { return mSwapChainExtent; }
		uint32_t width() { return mSwapChainExtent.width; }
		uint32_t height() { return mSwapChainExtent.height; }

		float extentAspectRatio() {	return static_cast<float>(mSwapChainExtent.width) / static_cast<float>(mSwapChainExtent.height); }
		VkFormat findDepthFormat();

		VkResult acquireNextImage(uint32_t* imageIndex);
		VkResult submitCommandBuffers(const VkCommandBuffer* buffers, uint32_t* imageIndex);

		bool compareSwapFormats(const VreSwapChain& swapchain) const 
		{
			return swapchain.mSwapChainDepthFormat == mSwapChainDepthFormat &&
				swapchain.mSwapChainImageFormat == mSwapChainImageFormat;
		}

	private:
		void init();
		void createSwapChain();
		void createImageViews();
		void createDepthResources();
		void createRenderPass();
		void createFramebuffers();
		void createSyncObjects();

		// Helper functions
		VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
		VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
		VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

		VkFormat mSwapChainImageFormat;
		VkFormat mSwapChainDepthFormat;
		VkExtent2D mSwapChainExtent;

		std::vector<VkFramebuffer> mSwapChainFramebuffers;
		VkRenderPass mRenderPass;

		std::vector<VkImage> mDepthImages;
		std::vector<VkDeviceMemory> mDepthImageMemorys;
		std::vector<VkImageView> mDepthImageViews;
		std::vector<VkImage> mSwapChainImages;
		std::vector<VkImageView> mSwapChainImageViews;

		VreDevice& mDevice;
		VkExtent2D mWindowExtent;

		VkSwapchainKHR mSwapChain;
		std::shared_ptr<VreSwapChain> mOldSwapChain;

		std::vector<VkSemaphore> mImageAvailableSemaphores;
		std::vector<VkSemaphore> mRenderFinishedSemaphores;
		std::vector<VkFence> mInFlightFences;
		std::vector<VkFence> mImagesInFlight;
		size_t mCurrentFrame = 0;
	};

} // namespace vre
