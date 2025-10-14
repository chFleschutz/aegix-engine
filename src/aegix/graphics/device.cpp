#include "pch.h"

#include "device.h"

#include "graphics/vulkan_tools.h"
#include "graphics/vulkan/volk_include.h"

#define VMA_IMPLEMENTATION
#define VMA_STATIC_VULKAN_FUNCTIONS 0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 0
#include <vk_mem_alloc.h>

namespace Aegix::Graphics
{
	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData)
	{
		ALOG::warn("Vulkan Validation: \n{}\n", pCallbackData->pMessage);
		return VK_FALSE;
	}

	static auto CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
		const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) -> VkResult
	{
		auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
		if (func != nullptr)
		{
			return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
		}

		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}

	static void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator)
	{
		auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
		if (func != nullptr)
		{
			func(instance, debugMessenger, pAllocator);
		}
	}

	// VulkanDevice --------------------------------------------------------------

	VulkanDevice::~VulkanDevice()
	{
		vkDestroyCommandPool(m_device, m_commandPool, nullptr);
		vmaDestroyAllocator(m_allocator);
		vkDestroyDevice(m_device, nullptr);

		if (ENABLE_VALIDATION)
		{
			DestroyDebugUtilsMessengerEXT(m_instance, m_debugMessenger, nullptr);
		}

		vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
		vkDestroyInstance(m_instance, nullptr);
	}

	void VulkanDevice::initialize(Core::Window& window)
	{
		createInstance();
		setupDebugUtils();
		createSurface(window);
		createPhysicalDevice();
		createLogicalDevice();
		createAllocator();
		createCommandPool();
	}

	auto VulkanDevice::beginSingleTimeCommands() const -> VkCommandBuffer
	{
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = m_commandPool;
		allocInfo.commandBufferCount = 1;

		VkCommandBuffer commandBuffer;
		vkAllocateCommandBuffers(m_device, &allocInfo, &commandBuffer);

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		vkBeginCommandBuffer(commandBuffer, &beginInfo);
		return commandBuffer;
	}

	void VulkanDevice::endSingleTimeCommands(VkCommandBuffer commandBuffer) const
	{
		vkEndCommandBuffer(commandBuffer);

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;

		vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(m_graphicsQueue);

		vkFreeCommandBuffers(m_device, m_commandPool, 1, &commandBuffer);
	}

	void VulkanDevice::createBuffer(VkBuffer& buffer, VmaAllocation& allocation, VkDeviceSize size,
		VkBufferUsageFlags bufferUsage, VmaAllocationCreateFlags allocFlags, VmaMemoryUsage memoryUsage) const
	{
		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = size;
		bufferInfo.usage = bufferUsage;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VmaAllocationCreateInfo allocInfo{};
		allocInfo.usage = memoryUsage;
		allocInfo.flags = allocFlags;

		VK_CHECK(vmaCreateBuffer(m_allocator, &bufferInfo, &allocInfo, &buffer, &allocation, nullptr));
	}

	void VulkanDevice::createImage(VkImage& image, VmaAllocation& allocation, const VkImageCreateInfo& imageInfo,
		const VmaAllocationCreateInfo& allocInfo) const
	{
		VK_CHECK(vmaCreateImage(m_allocator, &imageInfo, &allocInfo, &image, &allocation, nullptr));
	}

	auto VulkanDevice::querySwapChainSupport() const -> SwapChainSupportDetails
	{
		return querySwapChainSupport(m_physicalDevice);
	}

	auto VulkanDevice::findPhysicalQueueFamilies() const -> QueueFamilyIndices
	{
		return findQueueFamilies(m_physicalDevice);
	}

	auto VulkanDevice::findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling,
		VkFormatFeatureFlags features) const -> VkFormat
	{
		for (VkFormat format : candidates)
		{
			VkFormatProperties props;
			vkGetPhysicalDeviceFormatProperties(m_physicalDevice, format, &props);

			if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features)
			{
				return format;
			}
			else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features)
			{
				return format;
			}
		}

		AGX_ASSERT_X(false, "failed to find supported format!");
		return VK_FORMAT_UNDEFINED;
	}

	auto VulkanDevice::findAspectFlags(VkFormat format) const -> VkImageAspectFlags
	{
		switch (format)
		{
		case VK_FORMAT_D16_UNORM:
		case VK_FORMAT_X8_D24_UNORM_PACK32:
		case VK_FORMAT_D32_SFLOAT:
			return VK_IMAGE_ASPECT_DEPTH_BIT;

		case VK_FORMAT_S8_UINT:
			return VK_IMAGE_ASPECT_STENCIL_BIT;

		case VK_FORMAT_D16_UNORM_S8_UINT:
		case VK_FORMAT_D24_UNORM_S8_UINT:
		case VK_FORMAT_D32_SFLOAT_S8_UINT:
			return VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;

		default:
			return VK_IMAGE_ASPECT_COLOR_BIT;  // For color formats
		}
	}

	void VulkanDevice::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) const
	{
		VkCommandBuffer commandBuffer = beginSingleTimeCommands();

		VkBufferCopy copyRegion{};
		copyRegion.srcOffset = 0;
		copyRegion.dstOffset = 0;
		copyRegion.size = size;
		vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

		endSingleTimeCommands(commandBuffer);
	}

	void VulkanDevice::copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t layerCount) const
	{
		VkCommandBuffer commandBuffer = beginSingleTimeCommands();

		VkBufferImageCopy region{};
		region.bufferOffset = 0;
		region.bufferRowLength = 0;
		region.bufferImageHeight = 0;
		region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		region.imageSubresource.mipLevel = 0;
		region.imageSubresource.baseArrayLayer = 0;
		region.imageSubresource.layerCount = layerCount;
		region.imageOffset = { 0, 0, 0 };
		region.imageExtent = { width, height, 1 };

		vkCmdCopyBufferToImage(commandBuffer,
			buffer,
			image,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1,
			&region);

		endSingleTimeCommands(commandBuffer);
	}

	void VulkanDevice::transitionImageLayout(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout oldLayout,
		VkImageLayout newLayout, uint32_t mipLevels, VkImageAspectFlags aspectFlags) const
	{
		VkImageMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.oldLayout = oldLayout;
		barrier.newLayout = newLayout;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = image;
		barrier.subresourceRange.aspectMask = aspectFlags;
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = mipLevels;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;
		barrier.srcAccessMask = Tools::srcAccessMask(barrier.oldLayout);
		barrier.dstAccessMask = Tools::dstAccessMask(barrier.newLayout);

		VkPipelineStageFlags srcStage = Tools::srcStage(barrier.srcAccessMask);
		VkPipelineStageFlags dstStage = Tools::dstStage(barrier.dstAccessMask);

		vkCmdPipelineBarrier(commandBuffer,
			srcStage, dstStage,
			0,
			0, nullptr,
			0, nullptr,
			1, &barrier
		);
	}

	void VulkanDevice::transitionImageLayout(VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout,
		uint32_t mipLevels, VkImageAspectFlags aspectFlags) const
	{
		VkCommandBuffer commandBuffer = beginSingleTimeCommands();
		transitionImageLayout(commandBuffer, image, oldLayout, newLayout, mipLevels, aspectFlags);
		endSingleTimeCommands(commandBuffer);
	}

	void VulkanDevice::transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout,
		uint32_t mipLevels) const
	{
		transitionImageLayout(image, oldLayout, newLayout, mipLevels, findAspectFlags(format));
	}

	void VulkanDevice::createInstance()
	{
		VK_CHECK(volkInitialize());

		AGX_ASSERT_X(!ENABLE_VALIDATION || checkValidationLayerSupport(), "Validation layers requested, but not available!");

		VkApplicationInfo appInfo{};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "Aegix App";
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "Aegix Engine";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = API_VERSION;

		auto extensions = queryRequiredInstanceExtensions();

		VkInstanceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;
		createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
		createInfo.ppEnabledExtensionNames = extensions.data();
		createInfo.enabledLayerCount = 0;
		createInfo.pNext = nullptr;

		if constexpr (ENABLE_VALIDATION)
		{
			createInfo.enabledLayerCount = static_cast<uint32_t>(VALIDATION_LAYERS.size());
			createInfo.ppEnabledLayerNames = VALIDATION_LAYERS.data();

			VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo = debugMessengerCreateInfo();
			createInfo.pNext = &debugCreateInfo;
			ALOG::info("Vulkan Validation Layer enabled");
		}

		VK_CHECK(vkCreateInstance(&createInfo, nullptr, &m_instance));
		volkLoadInstance(m_instance);

		checkGflwRequiredInstanceExtensions();
		Tools::loadFunctionPointers(m_instance); // TODO: Remove this when volk is fully used
	}

	void VulkanDevice::setupDebugUtils()
	{
		if constexpr (ENABLE_VALIDATION)
		{
			VkDebugUtilsMessengerCreateInfoEXT createInfo = debugMessengerCreateInfo();
			VK_CHECK(CreateDebugUtilsMessengerEXT(m_instance, &createInfo, nullptr, &m_debugMessenger));
		}
	}

	void VulkanDevice::createSurface(Core::Window& window)
	{
		window.createSurface(m_instance, m_surface);
	}

	void VulkanDevice::createPhysicalDevice()
	{
		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(m_instance, &deviceCount, nullptr);

		AGX_ASSERT_X(deviceCount > 0, "Failed to find GPUs with Vulkan support");
		ALOG::info("Available GPUs: {}", deviceCount);

		std::vector<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(m_instance, &deviceCount, devices.data());

		// Search for a suitable device
		for (const auto& device : devices)
		{
			QueueFamilyIndices indices = findQueueFamilies(device);
			if (!indices.isComplete())
				continue;

			if (!checkDeviceExtensionSupport(device))
				continue;

			if (!checkDeviceFeatureSupport(device))
				continue;

			SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
			if (swapChainSupport.formats.empty() || swapChainSupport.presentModes.empty())
				continue;

			// Found a suitable device
			m_physicalDevice = device;
			break;
		}

		AGX_ASSERT_X(m_physicalDevice != VK_NULL_HANDLE, "Failed to find a suitable GPU");

		vkGetPhysicalDeviceProperties(m_physicalDevice, &m_properties);
		uint32_t major = VK_VERSION_MAJOR(m_properties.apiVersion);
		uint32_t minor = VK_VERSION_MINOR(m_properties.apiVersion);
		uint32_t patch = VK_VERSION_PATCH(m_properties.apiVersion);
		ALOG::info("{} (Vulkan {}.{}.{})", m_properties.deviceName, major, minor, patch);

		// Query features
		m_features = {};
		m_features.core.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
		m_features.core.pNext = &m_features.v11;
		m_features.v11.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES;
		m_features.v11.pNext = &m_features.v12;
		m_features.v12.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
		m_features.v12.pNext = &m_features.v13;
		m_features.v13.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
		m_features.v13.pNext = &m_features.meshShaderEXT;
		m_features.meshShaderEXT.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_EXT;

		vkGetPhysicalDeviceFeatures2(m_physicalDevice, &m_features.core);

		if (!m_features.meshShaderEXT.meshShader || !m_features.meshShaderEXT.taskShader)
			ALOG::warn("Mesh shaders not supported");
	}

	void VulkanDevice::createLogicalDevice()
	{
		VkPhysicalDeviceMeshShaderFeaturesEXT meshShader{
			.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_EXT,
			.taskShader = m_features.meshShaderEXT.taskShader,
			.meshShader = m_features.meshShaderEXT.meshShader,
		};

		VkPhysicalDeviceDynamicRenderingFeatures dynamicRendering{
			.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES,
			.pNext = &meshShader,
			.dynamicRendering = VK_TRUE,
		};

		VkPhysicalDeviceFeatures2 deviceFeatures{
			.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
			.pNext = &dynamicRendering,
		};
		deviceFeatures.features.samplerAnisotropy = VK_TRUE;

		QueueFamilyIndices indices = findQueueFamilies(m_physicalDevice);
		AGX_ASSERT_X(indices.isComplete(), "Queue family indices are not complete");

		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
		std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value() };

		float queuePriority = 1.0f;
		for (uint32_t queueFamily : uniqueQueueFamilies)
		{
			VkDeviceQueueCreateInfo queueCreateInfo{};
			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfo.queueFamilyIndex = queueFamily;
			queueCreateInfo.queueCount = 1;
			queueCreateInfo.pQueuePriorities = &queuePriority;
			queueCreateInfos.push_back(queueCreateInfo);
		}

		std::vector<const char*> enabledExtensions(DEVICE_EXTENSIONS.begin(), DEVICE_EXTENSIONS.end());
		if (m_features.meshShaderEXT.taskShader && m_features.meshShaderEXT.meshShader)
		{
			enabledExtensions.emplace_back(VK_EXT_MESH_SHADER_EXTENSION_NAME);
		}

		VkDeviceCreateInfo createInfo{
			.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
			.pNext = &deviceFeatures,
			.flags = 0,
			.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size()),
			.pQueueCreateInfos = queueCreateInfos.data(),
			.enabledLayerCount = 0,
			.ppEnabledLayerNames = nullptr,
			.enabledExtensionCount = static_cast<uint32_t>(enabledExtensions.size()),
			.ppEnabledExtensionNames = enabledExtensions.data(),
			.pEnabledFeatures = nullptr, 
		};

		// might not really be necessary anymore because device specific validation layers have been deprecated
		if constexpr (ENABLE_VALIDATION)
		{
			createInfo.enabledLayerCount = static_cast<uint32_t>(VALIDATION_LAYERS.size());
			createInfo.ppEnabledLayerNames = VALIDATION_LAYERS.data();
		}

		VK_CHECK(vkCreateDevice(m_physicalDevice, &createInfo, nullptr, &m_device));
		volkLoadDevice(m_device);

		AGX_ASSERT_X(indices.isComplete(), "Queue family indices are not complete");
		vkGetDeviceQueue(m_device, indices.graphicsFamily.value(), 0, &m_graphicsQueue);
		vkGetDeviceQueue(m_device, indices.presentFamily.value(), 0, &m_presentQueue);
	}

	void VulkanDevice::createAllocator()
	{
		VmaAllocatorCreateInfo allocatorInfo{
			.physicalDevice = m_physicalDevice,
			.device = m_device,
			.instance = m_instance,
			.vulkanApiVersion = API_VERSION
		};

		VmaVulkanFunctions vulkanFunctions;
		vmaImportVulkanFunctionsFromVolk(&allocatorInfo, &vulkanFunctions);
		allocatorInfo.pVulkanFunctions = &vulkanFunctions;

		VK_CHECK(vmaCreateAllocator(&allocatorInfo, &m_allocator));
	}

	void VulkanDevice::createCommandPool()
	{
		QueueFamilyIndices queueFamilyIndices = findPhysicalQueueFamilies();
		AGX_ASSERT_X(queueFamilyIndices.graphicsFamily.has_value(), "Graphics queue family not found");

		VkCommandPoolCreateInfo poolInfo = {};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
		poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

		VK_CHECK(vkCreateCommandPool(m_device, &poolInfo, nullptr, &m_commandPool))
	}

	auto VulkanDevice::debugMessengerCreateInfo() const -> VkDebugUtilsMessengerCreateInfoEXT
	{
		VkDebugUtilsMessengerCreateInfoEXT info{};
		info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		info.pfnUserCallback = debugCallback;

		return info;
	}

	auto VulkanDevice::checkValidationLayerSupport() -> bool
	{
		uint32_t layerCount;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

		std::vector<VkLayerProperties> availableLayers(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

		for (auto layerName : VALIDATION_LAYERS)
		{
			bool layerFound = false;
			for (const auto& layerProperties : availableLayers)
			{
				if (strcmp(layerName, layerProperties.layerName) == 0)
				{
					layerFound = true;
					break;
				}
			}

			if (!layerFound)
				return false;
		}

		return true;
	}

	auto VulkanDevice::queryRequiredInstanceExtensions() const -> std::vector<const char*>
	{
		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		std::vector<const char*> extensions;
		extensions.reserve(glfwExtensionCount);
		extensions.insert(extensions.end(), glfwExtensions, glfwExtensions + glfwExtensionCount);

		if constexpr (ENABLE_VALIDATION)
		{
			extensions.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		}

		return extensions;
	}

	void VulkanDevice::checkGflwRequiredInstanceExtensions()
	{
		uint32_t extensionCount = 0;
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
		std::vector<VkExtensionProperties> extensions(extensionCount);
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

		std::unordered_set<std::string> available;
		for (const auto& extension : extensions)
		{
			available.insert(extension.extensionName);
		}

		for (const auto& required : queryRequiredInstanceExtensions())
		{
			AGX_ASSERT_X(available.find(required) != available.end(), "Missing required glfw extension");
		}
	}

	auto VulkanDevice::checkDeviceExtensionSupport(VkPhysicalDevice device) -> bool
	{
		uint32_t extensionCount;
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

		std::vector<VkExtensionProperties> availableExtensions(extensionCount);
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

		std::set<std::string> requiredExtensions(DEVICE_EXTENSIONS.begin(), DEVICE_EXTENSIONS.end());
		for (const auto& extension : availableExtensions)
		{
			requiredExtensions.erase(extension.extensionName);
		}

		return requiredExtensions.empty();
	}

	auto VulkanDevice::checkDeviceFeatureSupport(VkPhysicalDevice device) -> bool
	{
		VkPhysicalDeviceFeatures features;
		vkGetPhysicalDeviceFeatures(device, &features);
		if (!features.samplerAnisotropy)
			return false;

		// TODO: check for more features (dynamic rendering, mesh shaders, etc.)

		return true;
	}

	auto VulkanDevice::findQueueFamilies(VkPhysicalDevice device) const -> QueueFamilyIndices
	{
		QueueFamilyIndices indices;

		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

		int i = 0;
		for (const auto& queueFamily : queueFamilies)
		{
			if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
				indices.graphicsFamily = i;

			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(device, i, m_surface, &presentSupport);
			if (queueFamily.queueCount > 0 && presentSupport)
				indices.presentFamily = i;

			if (indices.isComplete())
				break;

			i++;
		}

		return indices;
	}

	auto VulkanDevice::querySwapChainSupport(VkPhysicalDevice device) const -> SwapChainSupportDetails
	{
		SwapChainSupportDetails details;
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, m_surface, &details.capabilities);

		uint32_t formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_surface, &formatCount, nullptr);

		if (formatCount != 0)
		{
			details.formats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_surface, &formatCount, details.formats.data());
		}

		uint32_t presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_surface, &presentModeCount, nullptr);

		if (presentModeCount != 0)
		{
			details.presentModes.resize(presentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(
				device,
				m_surface,
				&presentModeCount,
				details.presentModes.data());
		}
		return details;
	}

}
