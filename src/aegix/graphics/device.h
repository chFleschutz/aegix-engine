#pragma once

#define ENGINE_DIR PROJECT_DIR "/"
#define SHADER_DIR ENGINE_DIR "shaders/"
#define ASSETS_DIR ENGINE_DIR "modules/aegix-assets/"

#include "graphics/deletion_queue.h"
#include "core/window.h"

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>

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
		static constexpr uint32_t API_VERSION = VK_API_VERSION_1_3;
		static constexpr auto VALIDATION_LAYERS = std::array{ "VK_LAYER_KHRONOS_validation" };
		static constexpr auto DEVICE_EXTENSIONS = std::array{ VK_KHR_SWAPCHAIN_EXTENSION_NAME };

		VulkanDevice(Core::Window& window);
		VulkanDevice(const VulkanDevice&) = delete;
		VulkanDevice(VulkanDevice&&) = delete;
		~VulkanDevice();

		auto operator=(const VulkanDevice&) -> VulkanDevice& = delete;
		auto operator=(VulkanDevice&&) -> VulkanDevice& = delete;

		operator VkDevice() { return m_device; }

		[[nodiscard]] auto instance() const -> VkInstance { return m_instance; }
		[[nodiscard]] auto physicalDevice() const -> VkPhysicalDevice { return m_physicalDevice; }
		[[nodiscard]] auto device() const -> VkDevice { return m_device; }
		[[nodiscard]] auto allocator() const -> VmaAllocator { return m_allocator; }
		[[nodiscard]] auto commandPool() const -> VkCommandPool { return m_commandPool; }
		[[nodiscard]] auto surface() const -> VkSurfaceKHR { return m_surface; }
		[[nodiscard]] auto graphicsQueue() const -> VkQueue { return m_graphicsQueue; }
		[[nodiscard]] auto presentQueue() const -> VkQueue { return m_presentQueue; }
		[[nodiscard]] auto properties() const -> const VkPhysicalDeviceProperties& { return m_properties; }

		auto querySwapChainSupport() const -> SwapChainSupportDetails;
		auto findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags m_properties) const -> uint32_t;
		auto findPhysicalQueueFamilies() const -> QueueFamilyIndices;
		auto findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, 
			VkFormatFeatureFlags features) const -> VkFormat;
		auto findAspectFlags(VkFormat format) const -> VkImageAspectFlags;

		auto beginSingleTimeCommands() const -> VkCommandBuffer;
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
		void setupDebugUtils();
		void createSurface(Core::Window& window);
		void createPhysicalDevice();
		void createLogicalDevice();
		void createAllocator();
		void createCommandPool();

		auto isDeviceSuitable(VkPhysicalDevice device) -> bool;
		auto queryRequiredInstanceExtensions() const -> std::vector<const char*>;
		auto checkValidationLayerSupport() -> bool;
		auto findQueueFamilies(VkPhysicalDevice device) const -> QueueFamilyIndices;
		auto debugMessengerCreateInfo() const -> VkDebugUtilsMessengerCreateInfoEXT;
		void checkGflwRequiredInstanceExtensions();
		auto checkDeviceExtensionSupport(VkPhysicalDevice device) -> bool;
		auto querySwapChainSupport(VkPhysicalDevice device) const -> SwapChainSupportDetails;

		VkInstance m_instance = VK_NULL_HANDLE;
		VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
		VkDevice m_device = VK_NULL_HANDLE;
		VmaAllocator m_allocator = VK_NULL_HANDLE;
		VkCommandPool m_commandPool = VK_NULL_HANDLE;

		VkDebugUtilsMessengerEXT m_debugMessenger;
		VkPhysicalDeviceProperties m_properties;

		VkSurfaceKHR m_surface;
		VkQueue m_graphicsQueue;
		VkQueue m_presentQueue;

		DeletionQueue m_deletionQueue;
	};
}
