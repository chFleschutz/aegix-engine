#pragma once

#define ENGINE_DIR PROJECT_DIR "/"
#define SHADER_DIR ENGINE_DIR "shaders/"
#define ASSETS_DIR ENGINE_DIR "assets/"
#define MODELS_DIR ENGINE_DIR "assets/models/"
#define TEXTURES_DIR ENGINE_DIR "assets/textures/"

#include "window.h"

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
#ifdef NDEBUG
		const bool enableValidationLayers = false;
#else
		const bool enableValidationLayers = true;
#endif

		VulkanDevice(Window& window);
		VulkanDevice(const VulkanDevice&) = delete;
		VulkanDevice(VulkanDevice&&) = delete;
		~VulkanDevice();

		VulkanDevice& operator=(const VulkanDevice&) = delete;
		VulkanDevice& operator=(VulkanDevice&&) = delete;

		VkInstance instance() { return m_instance; }
		VkCommandPool commandPool() { return m_commandPool; }
		VkDevice device() { return m_device; }
		VkSurfaceKHR surface() { return m_surface; }
		VkQueue graphicsQueue() { return m_graphicsQueue; }
		VkQueue presentQueue() { return m_presentQueue; }
		VkPhysicalDevice physicalDevice() { return m_physicalDevice; }
		VkPhysicalDeviceProperties properties() { return m_properties; }

		SwapChainSupportDetails querySwapChainSupport() { return querySwapChainSupport(m_physicalDevice); }
		uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags m_properties);
		QueueFamilyIndices findPhysicalQueueFamilies() { return findQueueFamilies(m_physicalDevice); }
		VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

		VkCommandBuffer beginSingleTimeCommands();
		void endSingleTimeCommands(VkCommandBuffer commandBuffer);

		void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags m_properties, VkBuffer& buffer, 
			VkDeviceMemory& bufferMemory);
		void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
		void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t layerCount);

		void createImage(const VkImageCreateInfo& imageInfo, VkMemoryPropertyFlags m_properties, VkImage& image, 
			VkDeviceMemory& imageMemory);
		void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, 
			uint32_t mipLevels = 1);

	private:
		void createInstance();
		void setupDebugMessenger();
		void createSurface();
		void pickPhysicalDevice();
		void createLogicalDevice();
		void createCommandPool();

		bool isDeviceSuitable(VkPhysicalDevice device);
		std::vector<const char*> getRequiredExtensions();
		bool checkValidationLayerSupport();
		QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
		void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
		void hasGflwRequiredInstanceExtensions();
		bool checkDeviceExtensionSupport(VkPhysicalDevice device);
		SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);

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
	};
}
