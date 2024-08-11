#pragma once

#include "graphics/device.h"
#include "graphics/render_system.h"
#include "graphics/swap_chain.h"
#include "graphics/systems/point_light_system.h"
#include "graphics/systems/simple_render_system.h"
#include "graphics/window.h"

#include <cassert>
#include <memory>
#include <vector>

namespace Aegix::Graphics
{
	class Renderer
	{
	public:
		Renderer(Window& window, VulkanDevice& device);
		Renderer(const Renderer&) = delete;
		Renderer& operator=(const Renderer&) = delete;
		~Renderer();

		VkRenderPass swapChainRenderPass() const { return m_swapChain->renderPass(); }
		float aspectRatio() const { return m_swapChain->extentAspectRatio(); }
		bool isFrameInProgress() const { return m_isFrameStarted; }
		VkCommandBuffer currentCommandBuffer() const;
		int frameIndex() const;
		
		VkCommandBuffer beginFrame();
		void endFrame();
		
		void beginSwapChainRenderPass(VkCommandBuffer commandBuffer);
		void endSwapChainRenderPass(VkCommandBuffer commandBuffer);

		void renderFrame(float frametime, Scene::Scene& scene);

		void shutdown();

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

		std::unique_ptr<Aegix::Graphics::DescriptorPool> m_globalPool;
		std::vector<std::unique_ptr<Buffer>> m_globalUniformBuffers;
		std::vector<VkDescriptorSet> m_globalDescriptorSets;

		std::unique_ptr<SimpleRenderSystem> m_simpleRenderSystem;
		std::unique_ptr<PointLightSystem> m_pointLightSystem;
		std::vector<std::unique_ptr<RenderSystem>> m_renderSystems;
	};
}
