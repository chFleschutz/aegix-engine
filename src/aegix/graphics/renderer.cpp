#include "pch.h"

#include "renderer.h"

#include "core/profiler.h"
#include "graphics/frame_graph/frame_graph_blackboard.h"
#include "graphics/render_context.h"
#include "graphics/render_passes/bloom_pass.h"
#include "graphics/render_passes/geometry_pass.h"
#include "graphics/render_passes/lighting_pass.h"
#include "graphics/render_passes/post_processing_pass.h"
#include "graphics/render_passes/present_pass.h"
#include "graphics/render_passes/sky_box_pass.h"
#include "graphics/render_passes/ssao_pass.h"
#include "graphics/render_passes/transparent_pass.h"
#include "graphics/render_passes/ui_pass.h"
#include "graphics/render_systems/static_mesh_render_system.h"
#include "graphics/vulkan_context.h"
#include "scene/scene.h"

namespace Aegix::Graphics
{
	Renderer::Renderer(Core::Window& window)
		: m_window{ window }, m_swapChain{ window.extent()}
	{
		createCommandBuffers();
		createFrameGraph();
	}

	Renderer::~Renderer()
	{
		vkFreeCommandBuffers(
			VulkanContext::device(),
			VulkanContext::device().commandPool(),
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
			m_swapChain.extent(),
			m_swapChain.aspectRatio()
		};

		RenderContext ctx{
			.scene = scene,
			.ui = ui,
			.cmd = commandBuffer,
			.frameIndex = m_currentFrameIndex
		};
		
		m_frameGraph.execute(frameInfo, ctx);

		endFrame(commandBuffer);
	}

	void Renderer::waitIdle()
	{
		vkDeviceWaitIdle(VulkanContext::device());
	}

	void Renderer::createCommandBuffers()
	{
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = VulkanContext::device().commandPool();
		allocInfo.commandBufferCount = static_cast<uint32_t>(m_commandBuffers.size());

		VK_CHECK(vkAllocateCommandBuffers(VulkanContext::device(), &allocInfo, m_commandBuffers.data()))
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

	void Renderer::createFrameGraph()
	{
		m_frameGraph.add<GeometryPass>(m_frameGraph);
		m_frameGraph.add<TransparentPass>(m_frameGraph);
		m_frameGraph.add<LightingPass>();
		m_frameGraph.add<PresentPass>(m_swapChain);
		m_frameGraph.add<UIPass>();
		m_frameGraph.add<PostProcessingPass>();
		m_frameGraph.add<BloomPass>();
		m_frameGraph.add<SSAOPass>();
		m_frameGraph.add<SkyBoxPass>();

		m_frameGraph.compile();
	}

	void Renderer::createRenderSystems()
	{
		m_renderSystemRegistry.add<StaticMeshRenderSystem>(RenderStageType::Opaque);
		m_renderSystemRegistry.add<StaticMeshRenderSystem>(RenderStageType::Transparent);
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
		VulkanContext::flushDeletionQueue(m_currentFrameIndex);

		m_isFrameStarted = false;
	}
}
