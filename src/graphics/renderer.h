#pragma once

#include "graphics/device.h"
#include "graphics/render_system.h"
#include "graphics/swap_chain.h"
#include "graphics/window.h"

#include <cassert>
#include <memory>
#include <vector>

namespace VEGraphics
{
	class Renderer
	{
	public:
		Renderer(Window& window, VulkanDevice& device);
		~Renderer();

		Renderer(const Renderer&) = delete;
		Renderer& operator=(const Renderer&) = delete;

		VkRenderPass swapChainRenderPass() const { return m_swapChain->renderPass(); }
		float aspectRatio() const { return m_swapChain->extentAspectRatio(); }
		bool isFrameInProgress() const { return m_isFrameStarted; }
		VkCommandBuffer currentCommandBuffer() const
		{
			assert(m_isFrameStarted && "Cannot get command buffer when frame not in progress");
			return m_commandBuffers[m_currentFrameIndex];
		}

		int frameIndex() const 
		{
			assert(m_isFrameStarted && "Cannot get frame index when frame not in progress");
			return m_currentFrameIndex;
		}

		VkCommandBuffer beginFrame();
		void endFrame();
		void beginSwapChainRenderPass(VkCommandBuffer commandBuffer);
		void endSwapChainRenderPass(VkCommandBuffer commandBuffer);

		void renderFrame();

	private:
		void createCommandBuffers();
		void freeCommandBuffers();
		void recreateSwapChain();

		Window& m_window;
		VulkanDevice& m_device;
		std::unique_ptr<SwapChain> m_swapChain;
		std::vector<VkCommandBuffer> m_commandBuffers;

		uint32_t m_currentImageIndex;
		int m_currentFrameIndex = 0;
		bool m_isFrameStarted = false;

		std::vector<std::unique_ptr<RenderSystem>> m_renderSystems;
	};

} // namespace vre
