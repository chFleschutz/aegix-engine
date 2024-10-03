#pragma once

#include "graphics/buffer.h"
#include "graphics/descriptors.h"
#include "graphics/device.h"
#include "graphics/frame_info.h"
#include "graphics/renderpasses/render_pass.h"
#include "graphics/swap_chain.h"
#include "graphics/systems/render_system.h"
#include "graphics/window.h"
#include "scene/scene.h"

#include <memory>
#include <type_traits>
#include <typeindex>
#include <unordered_map>
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

		template<typename T>
		RenderSystem& addRenderSystem()
		{
			static_assert(std::is_base_of_v<RenderSystem, T>, "T has to be a subclass of RenderSystem");
			auto it = m_renderSystems.find(typeid(T));
			if (it != m_renderSystems.end())
				return *it->second;

			auto newSystem = std::make_unique<T>(m_device, swapChainRenderPass(), m_globalSetLayout->descriptorSetLayout());
			return *m_renderSystems.emplace(typeid(T), std::move(newSystem)).first->second;
		}

		VulkanDevice& device() { return m_device; }
		DescriptorPool& globalPool() { return *m_globalPool; }
		VkRenderPass swapChainRenderPass() const { return m_swapChain->renderPass(); }
		float aspectRatio() const { return m_swapChain->extentAspectRatio(); }
		bool isFrameInProgress() const { return m_isFrameStarted; }
		VkCommandBuffer currentCommandBuffer() const;
		int frameIndex() const;

		VkCommandBuffer beginRenderFrame();
		void renderScene(VkCommandBuffer commandBuffer, Scene::Scene& scene);
		void endRenderFrame(VkCommandBuffer commandBuffer);

		void shutdown();

		void renderFrame(Scene::Scene& scene);

	private:
		void createCommandBuffers();
		void freeCommandBuffers();
		void recreateSwapChain();
		void initializeDescriptorPool();

		VkCommandBuffer beginFrame();
		void endFrame(VkCommandBuffer commandBuffer);

		void beginSwapChainRenderPass(VkCommandBuffer commandBuffer);
		void endSwapChainRenderPass(VkCommandBuffer commandBuffer);

		void initializeGlobalUBO();
		void updateGlobalUBO(const FrameInfo& frameInfo);

		Window& m_window;
		VulkanDevice& m_device;
		std::unique_ptr<SwapChain> m_swapChain;
		std::vector<VkCommandBuffer> m_commandBuffers;

		uint32_t m_currentImageIndex;
		int m_currentFrameIndex = 0;
		bool m_isFrameStarted = false;

		std::unique_ptr<DescriptorPool> m_globalPool;
		std::unique_ptr<DescriptorSetLayout> m_globalSetLayout;
		std::unique_ptr<DescriptorSet> m_globalDescriptorSet;
		std::unique_ptr<UniformBuffer<GlobalUbo>> m_globalUBO;

		std::unordered_map<std::type_index, std::unique_ptr<RenderSystem>> m_renderSystems;

		std::vector<RenderPass> m_renderpasses;
	};
}
