#pragma once

#include "core/window.h"
#include "graphics/deletion_queue.h"
#include "graphics/vulkan/debug_utils.h"
#include "graphics/vulkan/volk_include.h"

#include <vk_mem_alloc.h>

namespace Aegix::Graphics
{
	struct SwapChainSupportDetails
	{
		VkSurfaceCapabilitiesKHR capabilities{};
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
	};

	struct QueueFamilyIndices
	{
		std::optional<uint32_t> graphicsFamily;
		std::optional<uint32_t> presentFamily;
		bool isComplete() const { return graphicsFamily.has_value() && presentFamily.has_value(); }
	};

	struct VulkanFeatures
	{
		VkPhysicalDeviceFeatures2 core{};
		VkPhysicalDeviceVulkan11Features v11{};
		VkPhysicalDeviceVulkan12Features v12{};
		VkPhysicalDeviceVulkan13Features v13{};
		VkPhysicalDeviceMeshShaderFeaturesEXT meshShaderEXT{};
	};

	class VulkanDevice
	{
		friend class VulkanContext;

	public:
		static constexpr uint32_t API_VERSION = VK_API_VERSION_1_3;
		static constexpr auto VALIDATION_LAYERS = std::array{ "VK_LAYER_KHRONOS_validation" };
		static constexpr auto DEVICE_EXTENSIONS = std::array{ VK_KHR_SWAPCHAIN_EXTENSION_NAME };

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
		[[nodiscard]] auto features() const -> const VulkanFeatures& { return m_features; }

		void initialize(Core::Window& window);

		auto beginSingleTimeCommands() const->VkCommandBuffer;
		void endSingleTimeCommands(VkCommandBuffer commandBuffer) const;

		void createBuffer(VkBuffer& buffer, VmaAllocation& allocation, VkDeviceSize size, VkBufferUsageFlags bufferUsage, VmaAllocationCreateFlags allocFlags, VmaMemoryUsage memoryUsage) const;
		void createImage(VkImage& image, VmaAllocation& allocation, const VkImageCreateInfo& imageInfo, const VmaAllocationCreateInfo& allocInfo) const;

		auto querySwapChainSupport() const -> SwapChainSupportDetails;
		auto findPhysicalQueueFamilies() const -> QueueFamilyIndices;
		auto findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) const -> VkFormat;
		auto findAspectFlags(VkFormat format) const -> VkImageAspectFlags;

		void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) const;
		void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t layerCount) const;
		void transitionImageLayout(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout, 
			uint32_t mipLevels = 1, VkImageAspectFlags aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT) const; 
		void transitionImageLayout(VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels = 1, 
			VkImageAspectFlags aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT) const;
		void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, 
			uint32_t mipLevels = 1) const;

	private:
		VulkanDevice() = default;

		void createInstance();
		void createSurface(Core::Window& window);
		void createPhysicalDevice();
		void createLogicalDevice();
		void createAllocator();
		void createCommandPool();

		auto queryRequiredInstanceExtensions() const -> std::vector<const char*>;
		auto checkValidationLayerSupport() -> bool;
		auto findQueueFamilies(VkPhysicalDevice device) const -> QueueFamilyIndices;
		void checkGflwRequiredInstanceExtensions();
		auto checkDeviceExtensionSupport(VkPhysicalDevice device) -> bool;
		auto checkDeviceFeatureSupport(VkPhysicalDevice device) -> bool;
		auto querySwapChainSupport(VkPhysicalDevice device) const -> SwapChainSupportDetails;

		VkInstance m_instance = VK_NULL_HANDLE;
		VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
		VkDevice m_device = VK_NULL_HANDLE;
		VmaAllocator m_allocator = VK_NULL_HANDLE;
		VkCommandPool m_commandPool = VK_NULL_HANDLE;

		DebugUtilsMessenger m_debugMessenger;

		VkPhysicalDeviceProperties m_properties{};
		VulkanFeatures m_features{};

		VkSurfaceKHR m_surface = VK_NULL_HANDLE;
		VkQueue m_graphicsQueue = VK_NULL_HANDLE;
		VkQueue m_presentQueue = VK_NULL_HANDLE;
	};
}
