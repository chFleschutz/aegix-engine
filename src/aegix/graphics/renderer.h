#pragma once

#include "core/window.h"
#include "graphics/descriptors.h"
#include "graphics/device.h"
#include "graphics/frame_graph/frame_graph.h"
#include "graphics/globals.h"
#include "graphics/swap_chain.h"
#include "graphics/systems/render_system.h"
#include "scene/scene.h"

namespace Aegix::Graphics
{
	class Renderer
	{
	public:
		Renderer(Core::Window& window, VulkanDevice& device);
		Renderer(const Renderer&) = delete;
		Renderer(Renderer&&) = delete;
		~Renderer();

		auto operator=(const Renderer&) -> Renderer& = delete;
		auto operator=(Renderer&&) noexcept -> Renderer& = delete;

		template<ValidRenderSystem T>
		auto addRenderSystem() -> RenderSystem&
		{
			return m_frameGraph.resourcePool().addRenderSystem<T>(T::STAGE);
		}

		template<ValidMaterial T, typename... Args>
			requires std::constructible_from<typename T::Instance, DescriptorSetLayout&, DescriptorPool&, Args...>
		auto createMaterialInstance(Args&&... args) -> std::shared_ptr<typename T::Instance>
		{
			auto& system = addRenderSystem<typename T::RenderSystem>();
			return std::make_shared<typename T::Instance>(system.descriptorSetLayout(),
				*m_globalPool, std::forward<Args>(args)...);
		}

		[[nodiscard]] auto device() -> VulkanDevice& { return m_device; }
		[[nodiscard]] auto window() -> Core::Window& { return m_window; }
		[[nodiscard]] auto swapChain() -> SwapChain& { return m_swapChain; }
		[[nodiscard]] auto globalPool() -> DescriptorPool& { return *m_globalPool; }
		[[nodiscard]] auto frameGraph() -> FrameGraph& { return m_frameGraph; }
		[[nodiscard]] auto aspectRatio() const -> float { return m_swapChain.aspectRatio(); }
		[[nodiscard]] auto isFrameStarted() const -> bool { return m_isFrameStarted; }
		[[nodiscard]] auto currentCommandBuffer() const -> VkCommandBuffer;
		[[nodiscard]] auto frameIndex() const -> uint32_t;

		/// @brief Renders the given scene
		void renderFrame(Scene::Scene& scene, UI::UI& ui);

		/// @brief Waits for the GPU to be idle
		void waitIdle();

	private:
		void createCommandBuffers();
		void recreateSwapChain();
		void createDescriptorPool();
		void createFrameGraph();

		auto beginFrame() -> VkCommandBuffer;
		void endFrame(VkCommandBuffer commandBuffer);

		Core::Window& m_window;
		VulkanDevice& m_device;
		
		SwapChain m_swapChain;
		std::unique_ptr<DescriptorPool> m_globalPool;
		std::array<VkCommandBuffer, MAX_FRAMES_IN_FLIGHT> m_commandBuffers;

		int m_currentFrameIndex = 0;
		bool m_isFrameStarted = false;

		FrameGraph m_frameGraph;
	};
}
