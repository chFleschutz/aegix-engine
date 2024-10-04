#pragma once

#include "graphics/buffer.h"
#include "graphics/descriptors.h"
#include "graphics/device.h"
#include "graphics/frame_info.h"
#include "graphics/renderpasses/render_pass.h"
#include "graphics/swap_chain.h"
#include "graphics/systems/render_system.h"
#include "graphics/systems/render_system_collection.h"
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
			return m_renderSystemCollection.addRenderSystem<T>(m_device, swapChainRenderPass(), m_globalSetLayout->descriptorSetLayout());
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

		RenderSystemCollection m_renderSystemCollection;

		std::vector<std::unique_ptr<RenderPass>> m_renderpasses;
	};
}
