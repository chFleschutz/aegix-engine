#pragma once

#include "graphics/buffer.h"
#include "graphics/descriptors.h"
#include "graphics/device.h"
#include "graphics/swap_chain.h"
#include "graphics/window.h"

#include <memory>
#include <vector>
#include <unordered_map>
#include <typeindex>

namespace Aegix::Scene
{
	class Scene;
}

namespace Aegix::Graphics
{
	class RenderSystem;

	class PointLightSystem;
	class SimpleRenderSystem;

	template<typename T>
	struct RenderSystemRef;

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

		VkCommandBuffer beginFrame();
		void endFrame();
		
		void beginSwapChainRenderPass(VkCommandBuffer commandBuffer);
		void endSwapChainRenderPass(VkCommandBuffer commandBuffer);

		void renderFrame(float frametime, Scene::Scene* scene);

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

		std::unique_ptr<DescriptorPool> m_globalPool;
		std::vector<std::unique_ptr<Buffer>> m_globalUniformBuffers;
		std::unique_ptr<DescriptorSetLayout> m_globalSetLayout;
		std::vector<VkDescriptorSet> m_globalDescriptorSets;

		std::unique_ptr<SimpleRenderSystem> m_simpleRenderSystem;
		std::unique_ptr<PointLightSystem> m_pointLightSystem;
		std::unordered_map<std::type_index, std::unique_ptr<RenderSystem>> m_renderSystems;
	};
}
