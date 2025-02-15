#pragma once

#include "graphics/descriptors.h"
#include "graphics/device.h"
#include "graphics/frame_graph/frame_graph.h"
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
		Renderer(Renderer&&) = delete;
		~Renderer();

		Renderer& operator=(const Renderer&) = delete;
		Renderer& operator=(Renderer&&) = delete;

		template<typename T, typename... Args>
		RenderSystem& addRenderSystem(Args&&... args)
		{
			m_renderSystems.emplace_back(std::make_unique<T>(m_device, *m_globalSetLayout));
			return *m_renderSystems.back();
		}

		VulkanDevice& device() { return m_device; }
		SwapChain& swapChain() { return *m_swapChain; }
		DescriptorPool& globalPool() { return *m_globalPool; }
		VkCommandBuffer currentCommandBuffer() const;
		float aspectRatio() const { return m_swapChain->extentAspectRatio(); }
		bool isFrameStarted() const { return m_isFrameStarted; }
		int frameIndex() const;

		/// @brief Renders the given scene
		void renderFrame(Scene::Scene& scene);

		/// @brief Waits for the GPU to be idle
		void waitIdle();

	private:
		void createCommandBuffers();
		void recreateSwapChain();
		void createDescriptorPool();

		VkCommandBuffer beginFrame();
		void endFrame(VkCommandBuffer commandBuffer);

		void initializeGlobalUBO();
		void updateGlobalUBO(const FrameInfo& frameInfo);

		Window& m_window;
		VulkanDevice& m_device;
		
		std::unique_ptr<SwapChain> m_swapChain;
		std::unique_ptr<DescriptorPool> m_globalPool;
		std::array<VkCommandBuffer, SwapChain::MAX_FRAMES_IN_FLIGHT> m_commandBuffers;

		uint32_t m_currentImageIndex;
		int m_currentFrameIndex = 0;
		bool m_isFrameStarted = false;

		std::vector<std::unique_ptr<RenderSystem>> m_renderSystems;
		std::unique_ptr<DescriptorSetLayout> m_globalSetLayout;
		std::unique_ptr<DescriptorSet> m_globalSet;
		std::unique_ptr<UniformBuffer<GlobalUbo>> m_globalUBO;
	};
}
