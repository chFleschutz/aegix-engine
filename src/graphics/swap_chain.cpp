#include "swap_chain.h"

#include <array>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <cassert>

namespace Aegix::Graphics
{
	SwapChain::SwapChain(VulkanDevice& device, VkExtent2D windowExtent)
		: m_device{ device }, m_windowExtent{ windowExtent }
	{
		init();
	}

	SwapChain::SwapChain(VulkanDevice& device, VkExtent2D windowExtent, std::shared_ptr<SwapChain> previous)
		: m_device{ device }, m_windowExtent{ windowExtent }, m_oldSwapChain{ previous }
	{
		init();
		m_oldSwapChain = nullptr;
	}

	SwapChain::~SwapChain()
	{
		for (auto imageView : mColorImageViews)
		{
			vkDestroyImageView(m_device.device(), imageView, nullptr);
		}
		mColorImageViews.clear();

		if (m_swapChain != nullptr)
		{
			vkDestroySwapchainKHR(m_device.device(), m_swapChain, nullptr);
			m_swapChain = nullptr;
		}

		// cleanup synchronization objects
		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		{
			vkDestroySemaphore(m_device.device(), m_renderFinishedSemaphores[i], nullptr);
			vkDestroySemaphore(m_device.device(), m_imageAvailableSemaphores[i], nullptr);
			vkDestroyFence(m_device.device(), m_inFlightFences[i], nullptr);
		}
	}

	VkResult SwapChain::acquireNextImage(uint32_t* imageIndex)
	{
		vkWaitForFences(
			m_device.device(),
			1,
			&m_inFlightFences[m_currentFrame],
			VK_TRUE,
			std::numeric_limits<uint64_t>::max());

		VkResult result = vkAcquireNextImageKHR(
			m_device.device(),
			m_swapChain,
			std::numeric_limits<uint64_t>::max(),
			m_imageAvailableSemaphores[m_currentFrame],  // must be a not signaled semaphore
			VK_NULL_HANDLE,
			imageIndex);

		return result;
	}

	VkResult SwapChain::submitCommandBuffers(
		const VkCommandBuffer* buffers, uint32_t* imageIndex)
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
		if (vkQueueSubmit(m_device.graphicsQueue(), 1, &submitInfo, m_inFlightFences[m_currentFrame]) != VK_SUCCESS)
			throw std::runtime_error("failed to submit draw command buffer!");

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

	void SwapChain::transitionColorAttachment(VkCommandBuffer commandBuffer, uint32_t imageIndex)
	{
		VkImageMemoryBarrier imageBarrier{};
		imageBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		imageBarrier.srcAccessMask = 0;
		imageBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		imageBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageBarrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		imageBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imageBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imageBarrier.image = mColorImages[imageIndex];
		imageBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageBarrier.subresourceRange.baseMipLevel = 0;
		imageBarrier.subresourceRange.levelCount = 1;
		imageBarrier.subresourceRange.baseArrayLayer = 0;
		imageBarrier.subresourceRange.layerCount = 1;

		vkCmdPipelineBarrier(commandBuffer,
			VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			0,
			0, nullptr,
			0, nullptr,
			1, &imageBarrier
		);
	}

	void SwapChain::transitionPresent(VkCommandBuffer commandBuffer, uint32_t imageIndex)
	{
		VkImageMemoryBarrier presentBarrier{};
		presentBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		presentBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		presentBarrier.dstAccessMask = 0;
		presentBarrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		presentBarrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		presentBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		presentBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		presentBarrier.image = mColorImages[imageIndex];
		presentBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		presentBarrier.subresourceRange.baseMipLevel = 0;
		presentBarrier.subresourceRange.levelCount = 1;
		presentBarrier.subresourceRange.baseArrayLayer = 0;
		presentBarrier.subresourceRange.layerCount = 1;

		vkCmdPipelineBarrier(commandBuffer,
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
			0,
			0, nullptr,
			0, nullptr,
			1, &presentBarrier
		);
	}

	void SwapChain::init()
	{
		createSwapChain();
		createImageViews();
		createSyncObjects();
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
		createInfo.oldSwapchain = m_oldSwapChain == nullptr ? VK_NULL_HANDLE : m_oldSwapChain->m_swapChain;

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

		if (vkCreateSwapchainKHR(m_device.device(), &createInfo, nullptr, &m_swapChain) != VK_SUCCESS)
			throw std::runtime_error("Failed to create swap chain");

		vkGetSwapchainImagesKHR(m_device.device(), m_swapChain, &imageCount, nullptr);
		mColorImages.resize(imageCount);
		vkGetSwapchainImagesKHR(m_device.device(), m_swapChain, &imageCount, mColorImages.data());

		mSwapChainImageFormat = surfaceFormat.format;
		mSwapChainExtent = extent;
	}

	void SwapChain::createImageViews()
	{
		mColorImageViews.resize(mColorImages.size());
		for (size_t i = 0; i < mColorImages.size(); i++)
		{
			VkImageViewCreateInfo viewInfo{};
			viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			viewInfo.image = mColorImages[i];
			viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			viewInfo.format = mSwapChainImageFormat;
			viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			viewInfo.subresourceRange.baseMipLevel = 0;
			viewInfo.subresourceRange.levelCount = 1;
			viewInfo.subresourceRange.baseArrayLayer = 0;
			viewInfo.subresourceRange.layerCount = 1;

			if (vkCreateImageView(m_device.device(), &viewInfo, nullptr, &mColorImageViews[i]) != VK_SUCCESS)
				throw std::runtime_error("failed to create texture image view!");
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
			if (vkCreateSemaphore(m_device.device(), &semaphoreInfo, nullptr, &m_imageAvailableSemaphores[i]) != VK_SUCCESS ||
				vkCreateSemaphore(m_device.device(), &semaphoreInfo, nullptr, &m_renderFinishedSemaphores[i]) != VK_SUCCESS ||
				vkCreateFence(m_device.device(), &fenceInfo, nullptr, &m_inFlightFences[i]) != VK_SUCCESS)
			{
				throw std::runtime_error("failed to create synchronization objects for a frame!");
			}
		}
	}

	VkSurfaceFormatKHR SwapChain::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) const
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

	VkPresentModeKHR SwapChain::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) const 
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

	VkExtent2D SwapChain::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) const 
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

	uint32_t SwapChain::chooseImageCount(const VkSurfaceCapabilitiesKHR& capabilities) const
	{
		auto imageCount = capabilities.minImageCount + 1;
		if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount)
		{
			imageCount = capabilities.maxImageCount;
		}

		return imageCount;
	}

	VkFormat SwapChain::findDepthFormat()
	{
		// TODO: Rendersystems assume VK_FORMAT_D32_SFLOAT as the depth format
		return m_device.findSupportedFormat(
			{ VK_FORMAT_D32_SFLOAT/*, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT*/ },
			VK_IMAGE_TILING_OPTIMAL,
			VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
	}
}
