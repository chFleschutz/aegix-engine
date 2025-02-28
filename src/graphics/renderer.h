#pragma once

#include "graphics/descriptors.h"
#include "graphics/device.h"
#include "graphics/frame_graph/frame_graph.h"
#include "graphics/globals.h"
#include "graphics/swap_chain.h"
#include "graphics/systems/render_system.h"
#include "graphics/window.h"
#include "scene/scene.h"

#include <array>
#include <memory>

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

		template<typename T>
			requires std::is_base_of_v<RenderSystem, T> &&
			requires { T::STAGE; }
		RenderSystem& addRenderSystem()
		{
			constexpr RenderStage::Type stageType = T::STAGE;
			return m_frameGraph.resourcePool().addRenderSystem<T>(m_device, stageType);
		}

		VulkanDevice& device() { return m_device; }
		SwapChain& swapChain() { return m_swapChain; }
		DescriptorPool& globalPool() { return *m_globalPool; }
		VkCommandBuffer currentCommandBuffer() const;
		float aspectRatio() const { return m_swapChain.aspectRatio(); }
		bool isFrameStarted() const { return m_isFrameStarted; }
		int frameIndex() const;

		/// @brief Renders the given scene
		void renderFrame(Scene::Scene& scene, UI& gui);

		/// @brief Waits for the GPU to be idle
		void waitIdle();

	private:
		void createCommandBuffers();
		void recreateSwapChain();
		void createDescriptorPool();
		void createFrameGraph();

		VkCommandBuffer beginFrame();
		void endFrame(VkCommandBuffer commandBuffer);

		Window& m_window;
		VulkanDevice& m_device;
		
		SwapChain m_swapChain;
		std::unique_ptr<DescriptorPool> m_globalPool;
		std::array<VkCommandBuffer, MAX_FRAMES_IN_FLIGHT> m_commandBuffers;

		int m_currentFrameIndex = 0;
		bool m_isFrameStarted = false;

		FrameGraph m_frameGraph;
	};
}
