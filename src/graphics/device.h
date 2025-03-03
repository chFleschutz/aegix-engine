#pragma once

#define ENGINE_DIR PROJECT_DIR "/"
#define SHADER_DIR ENGINE_DIR "shaders/"
#define ASSETS_DIR ENGINE_DIR "modules/aegix-assets/"

#include "window.h"
#include "graphics/deletion_queue.h"

#include <string>
#include <vector>
#include <optional>

namespace Aegix::Graphics
{
	struct SwapChainSupportDetails
	{
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
	};

	struct QueueFamilyIndices
	{
		std::optional<uint32_t> graphicsFamily;
		std::optional<uint32_t> presentFamily;
		bool isComplete() const { return graphicsFamily.has_value() && presentFamily.has_value(); }
	};

	class VulkanDevice
	{
	public:
		VulkanDevice(Window& window);
		VulkanDevice(const VulkanDevice&) = delete;
		VulkanDevice(VulkanDevice&&) = delete;
		~VulkanDevice();

		VulkanDevice& operator=(const VulkanDevice&) = delete;
		VulkanDevice& operator=(VulkanDevice&&) = delete;

		operator VkDevice() { return m_device; }

		VkInstance instance() const { return m_instance; }
		VkCommandPool commandPool() const { return m_commandPool; }
		VkDevice device() const { return m_device; }
		VkSurfaceKHR surface() const { return m_surface; }
		VkQueue graphicsQueue() const { return m_graphicsQueue; }
		VkQueue presentQueue() const { return m_presentQueue; }
		VkPhysicalDevice physicalDevice() const { return m_physicalDevice; }
		VkPhysicalDeviceProperties properties() const { return m_properties; }

		SwapChainSupportDetails querySwapChainSupport() const;
		uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags m_properties) const;
		QueueFamilyIndices findPhysicalQueueFamilies() const;
		VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, 
			VkFormatFeatureFlags features) const;
		VkImageAspectFlags findAspectFlags(VkFormat format) const;

		VkCommandBuffer beginSingleTimeCommands() const;
		void endSingleTimeCommands(VkCommandBuffer commandBuffer) const;

		void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags m_properties, VkBuffer& buffer, 
			VkDeviceMemory& bufferMemory) const;
		void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) const;
		void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t layerCount) const;

		void createImage(const VkImageCreateInfo& imageInfo, VkMemoryPropertyFlags m_properties, VkImage& image, 
			VkDeviceMemory& imageMemory) const;
		void transitionImageLayout(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout, 
			uint32_t mipLevels = 1, VkImageAspectFlags aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT) const; 
		void transitionImageLayout(VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels = 1, 
			VkImageAspectFlags aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT) const;
		void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, 
			uint32_t mipLevels = 1) const;

		void flushDeletionQueue(uint32_t frameIndex) { m_deletionQueue.flush(frameIndex); }
		void scheduleDeletion(std::function<void()>&& func) { m_deletionQueue.schedule(std::move(func)); }
		void scheduleDeletion(VkImage image) { m_deletionQueue.schedule([=]() { vkDestroyImage(m_device, image, nullptr); }); }
		void scheduleDeletion(VkImageView view) { m_deletionQueue.schedule([=]() { vkDestroyImageView(m_device, view, nullptr); }); }
		void scheduleDeletion(VkBuffer buffer) { m_deletionQueue.schedule([=]() { vkDestroyBuffer(m_device, buffer, nullptr); }); }
		void scheduleDeletion(VkDeviceMemory memory) { m_deletionQueue.schedule([=]() { vkFreeMemory(m_device, memory, nullptr); }); }
		void scheduleDeletion(VkSampler sampler) { m_deletionQueue.schedule([=]() { vkDestroySampler(m_device, sampler, nullptr); }); }

	private:
		void createInstance();
		void setupDebugMessenger();
		void createSurface();
		void pickPhysicalDevice();
		void createLogicalDevice();
		void createCommandPool();

		bool isDeviceSuitable(VkPhysicalDevice device);
		std::vector<const char*> requiredExtensions() const;
		bool checkValidationLayerSupport();
		QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device) const;
		void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
		void hasGflwRequiredInstanceExtensions();
		bool checkDeviceExtensionSupport(VkPhysicalDevice device);
		SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device) const;

		VkInstance m_instance;
		VkDebugUtilsMessengerEXT m_debugMessenger;
		VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
		VkPhysicalDeviceProperties m_properties;

		Window& m_window;
		VkCommandPool m_commandPool;

		VkDevice m_device;
		VkSurfaceKHR m_surface;
		VkQueue m_graphicsQueue;
		VkQueue m_presentQueue;

		const std::vector<const char*> validationLayers = { "VK_LAYER_KHRONOS_validation" };
		const std::vector<const char*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

		DeletionQueue m_deletionQueue;
	};
}
