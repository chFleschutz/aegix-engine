#include "renderer.h"

#include "scene/scene.h"
#include "scene/entity.h"
#include "graphics/frame_graph/frame_graph_blackboard.h"
#include "graphics/render_passes/g_buffer_pass.h"
#include "graphics/render_passes/lighting_pass.h"

#include <cassert>
#include <stdexcept>

namespace Aegix::Graphics
{
	Renderer::Renderer(Window& window, VulkanDevice& device)
		: m_window{ window }, m_device{ device }
	{
		recreateSwapChain();
		createCommandBuffers();
		createDescriptorPool();
		createFrameGraph();
	}

	Renderer::~Renderer()
	{
		vkFreeCommandBuffers(
			m_device.device(),
			m_device.commandPool(),
			static_cast<uint32_t>(m_commandBuffers.size()),
			m_commandBuffers.data()
		);
	}

	VkCommandBuffer Graphics::Renderer::currentCommandBuffer() const
	{
		assert(m_isFrameStarted && "Cannot get command buffer when frame not in progress");
		assert(m_commandBuffers[m_currentFrameIndex] != VK_NULL_HANDLE && "Command buffer not initialized");

		return m_commandBuffers[m_currentFrameIndex];
	}

	int Graphics::Renderer::frameIndex() const
	{
		assert(m_isFrameStarted && "Cannot get frame index when frame not in progress");
		return m_currentFrameIndex;
	}

	void Renderer::renderFrame(Scene::Scene& scene)
	{
		auto commandBuffer = beginFrame();

		assert(commandBuffer && "Failed to begin frame");
		assert(m_isFrameStarted && "Frame not started");

		FrameInfo frameInfo{
			m_currentFrameIndex,
			commandBuffer,
			scene,
			m_swapChain->extentAspectRatio(),
			m_swapChain->extend(),
			m_swapChain->colorImageView(m_currentImageIndex),
			m_swapChain->depthImageView(m_currentImageIndex)
		};
		
		m_frameGraph.execute(frameInfo);

		endFrame(commandBuffer);
	}

	void Graphics::Renderer::waitIdle()
	{
		vkDeviceWaitIdle(m_device.device());
	}

	void Renderer::createCommandBuffers()
	{
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = m_device.commandPool();
		allocInfo.commandBufferCount = static_cast<uint32_t>(m_commandBuffers.size());

		if (vkAllocateCommandBuffers(m_device.device(), &allocInfo, m_commandBuffers.data()) != VK_SUCCESS)
			throw std::runtime_error("Failed to allocate command buffers");
	}

	void Renderer::recreateSwapChain()
	{
		auto extend = m_window.extend();
		while (extend.width == 0 || extend.height == 0) // minimized
		{
			glfwWaitEvents();
			extend = m_window.extend();
		}
		vkDeviceWaitIdle(m_device.device());

		if (m_swapChain)
		{
			std::shared_ptr<SwapChain> oldSwapChain = std::move(m_swapChain);
			m_swapChain = std::make_unique<SwapChain>(m_device, extend, oldSwapChain);

			assert(oldSwapChain->compareSwapFormats(*m_swapChain.get()) && "Swap chain image or depth format has changed");
		}
		else
		{
			m_swapChain = std::make_unique<SwapChain>(m_device, extend);
		}
	}

	void Renderer::createDescriptorPool()
	{
		// TODO: Let the pool grow dynamically (see: https://vkguide.dev/docs/extra-chapter/abstracting_descriptors/)
		m_globalPool = DescriptorPool::Builder(m_device)
			.setPoolFlags(VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT)
			.setMaxSets(1000)
			.addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000)
			.addPoolSize(VK_DESCRIPTOR_TYPE_SAMPLER, 500)
			.addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 4000)
			.addPoolSize(VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 4000)
			.addPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000)
			.addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000)
			.addPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000)
			.addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 2000)
			.addPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 2000)
			.addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000)
			.addPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000)
			.addPoolSize(VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 500)
			.build();
	}

	void Renderer::createFrameGraph()
	{
		FrameGraphBlackboard blackboard;
		blackboard.add<RendererData>(m_device, *m_globalPool);

		auto position = m_frameGraph.addTexture(m_device, "Position", { 1920, 1080, VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT });
		auto normal = m_frameGraph.addTexture(m_device, "Normal", { 1920, 1080, VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT });
		auto albedo = m_frameGraph.addTexture(m_device, "Albedo", { 1920, 1080, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT });
		auto arm = m_frameGraph.addTexture(m_device, "ARM", { 1920, 1080, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT });
		auto emissive = m_frameGraph.addTexture(m_device, "Emissive", { 1920, 1080, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT });
		auto depth = m_frameGraph.addTexture(m_device, "Depth", { 1920, 1080, VK_FORMAT_D32_SFLOAT, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT });

		auto sceneColor = m_frameGraph.addTexture(m_device, "SceneColor", { 1920, 1080, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT });

		GBufferPass{ m_frameGraph, blackboard, position, normal, albedo, arm, emissive, depth };
		LightingPass{ m_frameGraph, blackboard, sceneColor };

		m_frameGraph.compile();
	}

	VkCommandBuffer Renderer::beginFrame()
	{
		assert(!m_isFrameStarted && "Cannot call beginFrame while already in progress");

		auto result = m_swapChain->acquireNextImage(&m_currentImageIndex);
		if (result == VK_ERROR_OUT_OF_DATE_KHR)
		{
			recreateSwapChain();
			return nullptr;
		}

		if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
			throw std::runtime_error("Failed to aquire swap chain image");

		m_isFrameStarted = true;

		auto commandBuffer = currentCommandBuffer();
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
			throw std::runtime_error("Failed to begin recording command buffer");

		m_swapChain->transitionColorAttachment(commandBuffer, m_currentImageIndex);

		return commandBuffer;
	}

	void Renderer::endFrame(VkCommandBuffer commandBuffer)
	{
		assert(m_isFrameStarted && "Cannot call endFrame while frame is not in progress");

		m_swapChain->transitionPresent(commandBuffer, m_currentImageIndex);

		if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
			throw std::runtime_error("Failed to record command buffer");

		auto result = m_swapChain->submitCommandBuffers(&commandBuffer, &m_currentImageIndex);
		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_window.wasWindowResized())
		{
			m_window.resetWindowResizedFlag();
			recreateSwapChain();
		}
		else if (result != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to present swap chain image");
		}

		m_isFrameStarted = false;
		m_currentFrameIndex = (m_currentFrameIndex + 1) % SwapChain::MAX_FRAMES_IN_FLIGHT;
	}
}
