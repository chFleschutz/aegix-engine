#include "swap_chain.h"

#include "graphics/vulkan_tools.h"

#include <array>
#include <iostream>
#include <limits>
#include <cassert>

namespace Aegix::Graphics
{
	SwapChain::SwapChain(VulkanDevice& device, VkExtent2D windowExtent)
		: m_device{ device }, m_windowExtent{ windowExtent }
	{
		createSwapChain();
		createImageViews();
		createSyncObjects();
	}

	SwapChain::~SwapChain()
	{
		destroyImageViews();
		destroySwapChain(m_swapChain);

		// cleanup synchronization objects
		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		{
			vkDestroySemaphore(m_device.device(), m_renderFinishedSemaphores[i], nullptr);
			vkDestroySemaphore(m_device.device(), m_imageAvailableSemaphores[i], nullptr);
			vkDestroyFence(m_device.device(), m_inFlightFences[i], nullptr);
		}
	}

	auto SwapChain::acquireNextImage() -> VkResult
	{
		vkWaitForFences(m_device.device(), 
			1, 
			&m_inFlightFences[m_currentFrame],
			VK_TRUE, 
			std::numeric_limits<uint64_t>::max());

		VkResult result = vkAcquireNextImageKHR(m_device.device(),
			m_swapChain,
			std::numeric_limits<uint64_t>::max(),
			m_imageAvailableSemaphores[m_currentFrame],  // must be a not signaled semaphore
			VK_NULL_HANDLE,
			&m_currentImageIndex);

		return result;
	}

	auto SwapChain::submitCommandBuffers(const VkCommandBuffer* buffers, uint32_t* imageIndex) -> VkResult
	{
		if (m_imagesInFlight[*imageIndex] != VK_NULL_HANDLE)
			vkWaitForFences(m_device.device(), 1, &m_imagesInFlight[*imageIndex], VK_TRUE, UINT64_MAX);

		m_imagesInFlight[*imageIndex] = m_inFlightFences[m_currentFrame];

		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		VkSemaphore waitSemaphores[] = { m_imageAvailableSemaphores[m_currentFrame] };
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;

		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = buffers;

		VkSemaphore signalSemaphores[] = { m_renderFinishedSemaphores[m_currentFrame] };
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		vkResetFences(m_device.device(), 1, &m_inFlightFences[m_currentFrame]);
		VK_CHECK(vkQueueSubmit(m_device.graphicsQueue(), 1, &submitInfo, m_inFlightFences[m_currentFrame]));

		VkSwapchainKHR swapChains[] = { m_swapChain };
		VkPresentInfoKHR presentInfo = {};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphores;
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;
		presentInfo.pImageIndices = imageIndex;

		auto result = vkQueuePresentKHR(m_device.presentQueue(), &presentInfo);

		m_currentFrame = (m_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;

		return result;
	}

	void SwapChain::resize(VkExtent2D extent)
	{
		destroyImageViews();

		m_windowExtent = extent;

		createSwapChain();
		createImageViews();
	}

	void SwapChain::createSwapChain()
	{
		SwapChainSupportDetails swapChainSupport = m_device.querySwapChainSupport();
		VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
		VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
		VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);
		uint32_t imageCount = chooseImageCount(swapChainSupport.capabilities);

		VkSwapchainCreateInfoKHR createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = m_device.surface();
		createInfo.minImageCount = imageCount;
		createInfo.imageFormat = surfaceFormat.format;
		createInfo.imageColorSpace = surfaceFormat.colorSpace;
		createInfo.imageExtent = extent;
		createInfo.imageArrayLayers = 1;
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		createInfo.presentMode = presentMode;
		createInfo.clipped = VK_TRUE;
		createInfo.oldSwapchain = m_swapChain;

		QueueFamilyIndices indices = m_device.findPhysicalQueueFamilies();
		assert(indices.isComplete() && "Queue family indices are not complete");
		
		std::array<uint32_t, 2> queueFamilyIndices{ indices.graphicsFamily.value(), indices.presentFamily.value() };
		if (indices.graphicsFamily != indices.presentFamily)
		{
			createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			createInfo.queueFamilyIndexCount = queueFamilyIndices.size();
			createInfo.pQueueFamilyIndices = queueFamilyIndices.data();
		}
		else
		{
			createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			createInfo.queueFamilyIndexCount = 0;
			createInfo.pQueueFamilyIndices = nullptr;
		}

		VK_CHECK(vkCreateSwapchainKHR(m_device.device(), &createInfo, nullptr, &m_swapChain));
		destroySwapChain(createInfo.oldSwapchain);

		vkGetSwapchainImagesKHR(m_device.device(), m_swapChain, &imageCount, nullptr);
		m_images.resize(imageCount);
		vkGetSwapchainImagesKHR(m_device.device(), m_swapChain, &imageCount, m_images.data());

		m_format = surfaceFormat.format;
		m_extent = extent;

		// At the beginning of a frame all swapchain images should be in present layout
		for (const auto& image : m_images)
		{
			m_device.transitionImageLayout(image, m_format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
		}
	}

	void SwapChain::createImageViews()
	{
		m_imageViews.resize(m_images.size());
		for (size_t i = 0; i < m_images.size(); i++)
		{
			VkImageViewCreateInfo viewInfo{};
			viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			viewInfo.image = m_images[i];
			viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			viewInfo.format = m_format;
			viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			viewInfo.subresourceRange.baseMipLevel = 0;
			viewInfo.subresourceRange.levelCount = 1;
			viewInfo.subresourceRange.baseArrayLayer = 0;
			viewInfo.subresourceRange.layerCount = 1;

			VK_CHECK(vkCreateImageView(m_device.device(), &viewInfo, nullptr, &m_imageViews[i]));
		}
	}

	void SwapChain::createSyncObjects()
	{
		m_imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		m_renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		m_inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
		m_imagesInFlight.resize(imageCount(), VK_NULL_HANDLE);

		VkSemaphoreCreateInfo semaphoreInfo = {};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkFenceCreateInfo fenceInfo = {};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		{
			VK_CHECK(vkCreateSemaphore(m_device.device(), &semaphoreInfo, nullptr, &m_imageAvailableSemaphores[i]));
			VK_CHECK(vkCreateSemaphore(m_device.device(), &semaphoreInfo, nullptr, &m_renderFinishedSemaphores[i]));
			VK_CHECK(vkCreateFence(m_device.device(), &fenceInfo, nullptr, &m_inFlightFences[i]));
		}
	}

	void SwapChain::destroyImageViews()
	{
		for (auto imageView : m_imageViews)
		{
			vkDestroyImageView(m_device.device(), imageView, nullptr);
		}
		m_imageViews.clear();
	}

	void SwapChain::destroySwapChain(VkSwapchainKHR swapChain)
	{
		if (swapChain)
		{
			vkDestroySwapchainKHR(m_device.device(), swapChain, nullptr);
		}
	}

	auto SwapChain::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) const -> VkSurfaceFormatKHR
	{
		for (const auto& availableFormat : availableFormats)
		{
			if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && 
				availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
			{
				return availableFormat;
			}
		}

		// TODO: Rendersystems assume VK_FORMAT_B8G8R8A8_SRGB as the swapchain format
		assert(availableFormats[0].format == VK_FORMAT_B8G8R8A8_SRGB && "Currently the only supported format is VK_FORMAT_B8G8R8A8_SRGB");
		return availableFormats[0];
	}

	auto SwapChain::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) const -> VkPresentModeKHR
	{
		for (const auto& availablePresentMode : availablePresentModes)
		{
			if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
			{
				std::cout << "Present mode: Mailbox" << std::endl;
				return availablePresentMode;
			}
		}

		std::cout << "Present mode: V-Sync" << std::endl;
		return VK_PRESENT_MODE_FIFO_KHR;
	}

	auto SwapChain::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) const -> VkExtent2D
	{
		if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
		{
			return capabilities.currentExtent;
		}
		else
		{
			VkExtent2D actualExtent = m_windowExtent;
			actualExtent.width = std::max(
				capabilities.minImageExtent.width,
				std::min(capabilities.maxImageExtent.width, actualExtent.width));
			actualExtent.height = std::max(
				capabilities.minImageExtent.height,
				std::min(capabilities.maxImageExtent.height, actualExtent.height));

			return actualExtent;
		}
	}

	auto SwapChain::chooseImageCount(const VkSurfaceCapabilitiesKHR& capabilities) const -> uint32_t
	{
		auto imageCount = capabilities.minImageCount + 1;
		if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount)
		{
			imageCount = capabilities.maxImageCount;
		}

		return imageCount;
	}

	auto SwapChain::findDepthFormat() -> VkFormat
	{
		// TODO: Rendersystems assume VK_FORMAT_D32_SFLOAT as the depth format
		return m_device.findSupportedFormat(
			{ VK_FORMAT_D32_SFLOAT/*, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT*/ },
			VK_IMAGE_TILING_OPTIMAL,
			VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
	}
}
