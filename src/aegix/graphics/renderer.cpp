#include "pch.h"

#include "renderer.h"

#include "core/profiler.h"
#include "graphics/frame_graph/frame_graph_blackboard.h"
#include "graphics/render_passes/bloom_pass.h"
#include "graphics/render_passes/geometry_pass.h"
#include "graphics/render_passes/lighting_pass.h"
#include "graphics/render_passes/post_processing_pass.h"
#include "graphics/render_passes/present_pass.h"
#include "graphics/render_passes/sky_box_pass.h"
#include "graphics/render_passes/ssao_pass.h"
#include "graphics/render_passes/transparent_pass.h"
#include "graphics/render_passes/ui_pass.h"
#include "scene/scene.h"

namespace Aegix::Graphics
{
	Renderer::Renderer(Core::Window& window, VulkanDevice& device)
		: m_window{ window }, m_device{ device }, m_swapChain{ device, window.extent() }
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

	auto Renderer::currentCommandBuffer() const -> VkCommandBuffer
	{
		AGX_ASSERT_X(m_isFrameStarted, "Cannot get command buffer when frame not in progress");
		AGX_ASSERT_X(m_commandBuffers[m_currentFrameIndex] != VK_NULL_HANDLE, "Command buffer not initialized");

		return m_commandBuffers[m_currentFrameIndex];
	}

	auto Renderer::frameIndex() const -> uint32_t
	{
		AGX_ASSERT_X(m_isFrameStarted, "Cannot get frame index when frame not in progress");
		return m_currentFrameIndex;
	}

	void Renderer::renderFrame(Scene::Scene& scene, UI::UI& ui)
	{
		AGX_PROFILE_FUNCTION();

		auto commandBuffer = beginFrame();

		AGX_ASSERT_X(commandBuffer, "Failed to begin frame");
		AGX_ASSERT_X(m_isFrameStarted, "Frame not started");

		FrameInfo frameInfo{
			scene,
			ui,
			m_currentFrameIndex,
			commandBuffer,
			m_swapChain.extent(),
			m_swapChain.aspectRatio()
		};
		
		m_frameGraph.execute(frameInfo);

		endFrame(commandBuffer);
	}

	void Renderer::waitIdle()
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
		VkExtent2D extent = m_window.extent();
		while (extent.width == 0 || extent.height == 0) // minimized
		{
			glfwWaitEvents();
			extent = m_window.extent();
		}
		
		waitIdle();
		m_swapChain.resize(extent);
		m_frameGraph.swapChainResized(extent.width, extent.height);
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
		m_frameGraph.add<UIPass>();
		m_frameGraph.add<PostProcessingPass>(m_device, *m_globalPool);
		m_frameGraph.add<BloomPass>(m_device, *m_globalPool);
		m_frameGraph.add<SSAOPass>(m_device, *m_globalPool);
		m_frameGraph.add<SkyBoxPass>(m_device, *m_globalPool);

		m_frameGraph.compile(m_device);
	}

	auto Renderer::beginFrame() -> VkCommandBuffer
	{
		AGX_ASSERT(!m_isFrameStarted && "Cannot call beginFrame while already in progress");

		auto result = m_swapChain.acquireNextImage();
		if (result == VK_ERROR_OUT_OF_DATE_KHR)
		{
			recreateSwapChain();
			return nullptr;
		}

		AGX_ASSERT(result == VK_SUCCESS || result == VK_SUBOPTIMAL_KHR && "Failed to aquire swap chain image");

		m_isFrameStarted = true;

		auto commandBuffer = currentCommandBuffer();
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		VK_CHECK(vkBeginCommandBuffer(commandBuffer, &beginInfo))

		return commandBuffer;
	}

	void Renderer::endFrame(VkCommandBuffer commandBuffer)
	{
		AGX_ASSERT(m_isFrameStarted && "Cannot call endFrame while frame is not in progress");

		VK_CHECK(vkEndCommandBuffer(commandBuffer))

		auto result = m_swapChain.submitCommandBuffers(&commandBuffer);
		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_window.wasResized())
		{
			m_window.resetResizedFlag();
			recreateSwapChain();
		}
		else if (result != VK_SUCCESS)
		{
			AGX_ASSERT(false && "Failed to present swap chain image");
		}

		waitIdle();

		m_currentFrameIndex = (m_currentFrameIndex + 1) % MAX_FRAMES_IN_FLIGHT;
		m_device.flushDeletionQueue(m_currentFrameIndex);

		m_isFrameStarted = false;
	}
}
