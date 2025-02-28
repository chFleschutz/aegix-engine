#include "renderer.h"

#include "graphics/frame_graph/frame_graph_blackboard.h"
#include "graphics/render_passes/bloom_pass.h"
#include "graphics/render_passes/geometry_pass.h"
#include "graphics/render_passes/gui_pass.h"
#include "graphics/render_passes/lighting_pass.h"
#include "graphics/render_passes/post_processing_pass.h"
#include "graphics/render_passes/present_pass.h"
#include "graphics/render_passes/transparent_pass.h"
#include "scene/scene.h"

#include <cassert>

namespace Aegix::Graphics
{
	Renderer::Renderer(Window& window, VulkanDevice& device)
		: m_window{ window }, m_device{ device }, m_swapChain{ device, window.extend() }
	{
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

	void Renderer::renderFrame(Scene::Scene& scene, GUI& gui)
	{
		auto commandBuffer = beginFrame();

		assert(commandBuffer && "Failed to begin frame");
		assert(m_isFrameStarted && "Frame not started");

		FrameInfo frameInfo{
			scene,
			gui,
			m_currentFrameIndex,
			commandBuffer,
			m_swapChain.extent(),
			m_swapChain.aspectRatio()
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

		VK_CHECK(vkAllocateCommandBuffers(m_device.device(), &allocInfo, m_commandBuffers.data()))
	}

	void Renderer::recreateSwapChain()
	{
		VkExtent2D extend = m_window.extend();
		while (extend.width == 0 || extend.height == 0) // minimized
		{
			glfwWaitEvents();
			extend = m_window.extend();
		}
		
		waitIdle();
		m_swapChain.resize(extend);

		m_frameGraph.swapChainResized(extend.width, extend.height);
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
		m_frameGraph.add<GeometryPass>(m_frameGraph, m_device, *m_globalPool);
		m_frameGraph.add<TransparentPass>(m_frameGraph, m_device, *m_globalPool);
		m_frameGraph.add<LightingPass>(m_device, *m_globalPool);
		m_frameGraph.add<PresentPass>(m_swapChain);
		m_frameGraph.add<GUIPass>();
		m_frameGraph.add<PostProcessingPass>(m_device, *m_globalPool);
		m_frameGraph.add<BloomPass>(m_device, *m_globalPool);

		m_frameGraph.compile(m_device);
	}

	VkCommandBuffer Renderer::beginFrame()
	{
		assert(!m_isFrameStarted && "Cannot call beginFrame while already in progress");

		auto result = m_swapChain.acquireNextImage();
		if (result == VK_ERROR_OUT_OF_DATE_KHR)
		{
			recreateSwapChain();
			return nullptr;
		}

		assert(result == VK_SUCCESS || result == VK_SUBOPTIMAL_KHR && "Failed to aquire swap chain image");

		m_isFrameStarted = true;

		auto commandBuffer = currentCommandBuffer();
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		VK_CHECK(vkBeginCommandBuffer(commandBuffer, &beginInfo))

		return commandBuffer;
	}

	void Renderer::endFrame(VkCommandBuffer commandBuffer)
	{
		assert(m_isFrameStarted && "Cannot call endFrame while frame is not in progress");

		VK_CHECK(vkEndCommandBuffer(commandBuffer))

		auto result = m_swapChain.submitCommandBuffers(&commandBuffer);
		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_window.wasWindowResized())
		{
			m_window.resetWindowResizedFlag();
			recreateSwapChain();
		}
		else if (result != VK_SUCCESS)
		{
			assert(false && "Failed to present swap chain image");
		}

		waitIdle();

		m_currentFrameIndex = (m_currentFrameIndex + 1) % MAX_FRAMES_IN_FLIGHT;
		m_device.flushDeletionQueue(m_currentFrameIndex);

		m_isFrameStarted = false;
	}
}
