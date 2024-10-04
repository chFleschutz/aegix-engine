#pragma once

#include "graphics/descriptors.h"
#include "graphics/device.h"
#include "graphics/renderpasses/render_pass.h"
#include "graphics/swap_chain.h"
#include "graphics/systems/render_system.h"
#include "graphics/window.h"
#include "scene/scene.h"

#include <array>
#include <memory>
#include <vector>

namespace Aegix::Graphics
{
	class Renderer
	{
	public:
		Renderer(Window& window, VulkanDevice& device);
		Renderer(const Renderer&) = delete;
		~Renderer();

		Renderer& operator=(const Renderer&) = delete;

		template<typename T>
		RenderSystem& addRenderSystem(int renderPassIndex = 0)
		{
			// TODO:
			// Add parameter for renderpass identifier
			// Find renderpass by identifier
			// Add render system to this renderpass
			return m_renderpasses[renderPassIndex]->addRenderSystem<T>(m_device, swapChainRenderPass());
		}

		VulkanDevice& device() { return m_device; }
		DescriptorPool& globalPool() { return *m_globalPool; }
		VkRenderPass swapChainRenderPass() const { return m_swapChain->renderPass(); }
		VkCommandBuffer currentCommandBuffer() const;
		float aspectRatio() const { return m_swapChain->extentAspectRatio(); }
		bool isFrameInProgress() const { return m_isFrameStarted; }
		int frameIndex() const;

		/// @brief Renders the given scene
		void renderFrame(Scene::Scene& scene);

		/// @brief Waits for the GPU to be idle
		void waitIdle();

	private:
		void createCommandBuffers();
		void freeCommandBuffers();
		void recreateSwapChain();
		void initializeDescriptorPool();

		VkCommandBuffer beginFrame();
		void endFrame(VkCommandBuffer commandBuffer);

		void beginSwapChainRenderPass(VkCommandBuffer commandBuffer);
		void endSwapChainRenderPass(VkCommandBuffer commandBuffer);

		Window& m_window;
		VulkanDevice& m_device;
		std::unique_ptr<SwapChain> m_swapChain;
		std::array<VkCommandBuffer, SwapChain::MAX_FRAMES_IN_FLIGHT> m_commandBuffers;

		uint32_t m_currentImageIndex;
		int m_currentFrameIndex = 0;
		bool m_isFrameStarted = false;

		std::unique_ptr<DescriptorPool> m_globalPool;

		std::vector<std::unique_ptr<RenderPass>> m_renderpasses;
	};
}
