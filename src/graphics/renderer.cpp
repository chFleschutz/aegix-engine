#include "renderer.h"

#include "graphics/renderpasses/lighting_pass.h"
#include "graphics/renderpasses/ui_pass.h"
#include "scene/scene.h"

#include <cassert>
#include <stdexcept>

namespace Aegix::Graphics
{
	Renderer::Renderer(Window& window, VulkanDevice& device)
		: m_window{ window }, m_device{ device }
	{
		recreateSwapChain();
		createCommandBuffers();
		initializeDescriptorPool();

		m_renderpasses.emplace_back(std::make_unique<LightingPass>(m_device, *m_globalPool));
		m_renderpasses.emplace_back(std::make_unique<UiPass>(m_window, m_device, m_globalPool->descriptorPool(), swapChainRenderPass()));
	}

	Renderer::~Renderer()
	{
		freeCommandBuffers();
	}

	VkCommandBuffer Graphics::Renderer::currentCommandBuffer() const
	{
		assert(m_isFrameStarted && "Cannot get command buffer when frame not in progress");
		assert(m_commandBuffers[m_currentFrameIndex] != VK_NULL_HANDLE && "Command buffer not initialized");

		return m_commandBuffers[m_currentFrameIndex];
	}

	int Graphics::Renderer::frameIndex() const
	{
		assert(m_isFrameStarted && "Cannot get frame index when frame not in progress");
		return m_currentFrameIndex;
	}

	void Renderer::renderFrame(Scene::Scene& scene)
	{
		auto commandBuffer = beginFrame();
		beginSwapChainRenderPass(commandBuffer);

		FrameInfo frameInfo{
			m_currentFrameIndex,
			commandBuffer,
			scene,
			m_swapChain->extentAspectRatio(),
		};

		for (auto& renderpass : m_renderpasses)
		{
			renderpass->render(frameInfo);
		}

		endSwapChainRenderPass(commandBuffer);
		endFrame(commandBuffer);
	}

	void Graphics::Renderer::waitIdle()
	{
		vkDeviceWaitIdle(m_device.device());
	}

	void Renderer::createCommandBuffers()
	{
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = m_device.commandPool();
		allocInfo.commandBufferCount = static_cast<uint32_t>(m_commandBuffers.size());

		if (vkAllocateCommandBuffers(m_device.device(), &allocInfo, m_commandBuffers.data()) != VK_SUCCESS)
			throw std::runtime_error("Failed to allocate command buffers");
	}

	void Renderer::freeCommandBuffers()
	{
		vkFreeCommandBuffers(
			m_device.device(),
			m_device.commandPool(),
			static_cast<uint32_t>(m_commandBuffers.size()),
			m_commandBuffers.data()
		);

		std::fill(m_commandBuffers.begin(), m_commandBuffers.end(), VK_NULL_HANDLE);
	}

	void Renderer::recreateSwapChain()
	{
		auto extend = m_window.extend();
		while (extend.width == 0 || extend.height == 0) // minimized
		{
			extend = m_window.extend();
			glfwWaitEvents();
		}
		vkDeviceWaitIdle(m_device.device());

		if (m_swapChain == nullptr)
		{
			m_swapChain = std::make_unique<SwapChain>(m_device, extend);
		}
		else
		{
			std::shared_ptr<SwapChain> oldSwapChain = std::move(m_swapChain);
			m_swapChain = std::make_unique<SwapChain>(m_device, extend, oldSwapChain);
			if (!oldSwapChain->compareSwapFormats(*m_swapChain.get()))
				throw std::runtime_error("Swap chain image or depth format has changed");
		}
	}

	void Renderer::initializeDescriptorPool()
	{
		// TODO: Let the pool grow dynamically (see: https://vkguide.dev/docs/extra-chapter/abstracting_descriptors/)
		m_globalPool = DescriptorPool::Builder(m_device)
			.setPoolFlags(VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT)
			.setMaxSets(1000)
			.addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000)
			.addPoolSize(VK_DESCRIPTOR_TYPE_SAMPLER, 500)
			.addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 4000)
			.addPoolSize(VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 4000)
			.addPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000)
			.addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000)
			.addPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000)
			.addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 2000)
			.addPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 2000)
			.addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000)
			.addPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000)
			.addPoolSize(VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 500)
			.build();
	}

	VkCommandBuffer Renderer::beginFrame()
	{
		assert(!m_isFrameStarted && "Cannot call beginFrame while already in progress");

		auto result = m_swapChain->acquireNextImage(&m_currentImageIndex);
		if (result == VK_ERROR_OUT_OF_DATE_KHR)
		{
			recreateSwapChain();
			return nullptr;
		}

		if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
			throw std::runtime_error("Failed to aquire swap chain image");

		m_isFrameStarted = true;

		auto commandBuffer = currentCommandBuffer();
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
			throw std::runtime_error("Failed to begin recording command buffer");

		return commandBuffer;
	}

	void Renderer::endFrame(VkCommandBuffer commandBuffer)
	{
		assert(m_isFrameStarted && "Cannot call endFrame while frame is not in progress");

		if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
			throw std::runtime_error("Failed to record command buffer");

		auto result = m_swapChain->submitCommandBuffers(&commandBuffer, &m_currentImageIndex);
		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_window.wasWindowResized())
		{
			m_window.resetWindowResizedFlag();
			recreateSwapChain();
		}
		else if (result != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to present swap chain image");
		}

		m_isFrameStarted = false;
		m_currentFrameIndex = (m_currentFrameIndex + 1) % SwapChain::MAX_FRAMES_IN_FLIGHT;
	}

	void Renderer::beginSwapChainRenderPass(VkCommandBuffer commandBuffer)
	{
		assert(m_isFrameStarted && "Cannot call beginSwapChainRenderPass while frame is not in progress");
		assert(commandBuffer == currentCommandBuffer() && "Cannot begin render pass on a command buffer from a diffrent frame");

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = m_swapChain->renderPass();
		renderPassInfo.framebuffer = m_swapChain->frameBuffer(m_currentImageIndex);

		renderPassInfo.renderArea.offset = { 0,0 };
		renderPassInfo.renderArea.extent = { m_swapChain->swapChainExtent() };

		std::array<VkClearValue, 2> clearValues{};
		clearValues[0].color = { 0.0f, 0.0f, 0.0f, 1.0f };
		clearValues[1].depthStencil = { 1.0f, 0 };
		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassInfo.pClearValues = clearValues.data();

		vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(m_swapChain->swapChainExtent().width);
		viewport.height = static_cast<float>(m_swapChain->swapChainExtent().height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		VkRect2D scissor{ {0,0}, m_swapChain->swapChainExtent() };
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
	}

	void Renderer::endSwapChainRenderPass(VkCommandBuffer commandBuffer)
	{
		assert(m_isFrameStarted && "Cannot call endSwapChainRenderPass while frame is not in progress");
		assert(commandBuffer == currentCommandBuffer() && "Cannot end render pass on a command buffer from a diffrent frame");

		vkCmdEndRenderPass(commandBuffer);
	}
}
