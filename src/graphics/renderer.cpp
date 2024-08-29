#include "renderer.h"

#include "scene/components.h"
#include "scene/entity.h"
#include "scene/scene.h"

#include <array>
#include <cassert>
#include <stdexcept>

//temp
#include <glm/gtc/matrix_transform.hpp>

namespace Aegix::Graphics
{
	Renderer::Renderer(Window& window, VulkanDevice& device) 
		: m_window{ window }, m_device{ device }
	{
		recreateSwapChain();
		createCommandBuffers();
		initializeDescriptorPool();
		initializeGlobalUBO();
	}

	Renderer::~Renderer()
	{
		freeCommandBuffers();
	}

	VkCommandBuffer Graphics::Renderer::currentCommandBuffer() const
	{
		assert(m_isFrameStarted && "Cannot get command buffer when frame not in progress");
		return m_commandBuffers[m_currentFrameIndex];
	}

	int Graphics::Renderer::frameIndex() const
	{
		assert(m_isFrameStarted && "Cannot get frame index when frame not in progress");
		return m_currentFrameIndex;
	}

	VkCommandBuffer Renderer::beginRenderFrame()
	{
		auto commandBuffer = beginFrame();
		assert(commandBuffer && "Failed to begin frame");

		beginSwapChainRenderPass(commandBuffer);
		return commandBuffer;
	}

	void Renderer::renderScene(VkCommandBuffer commandBuffer, Scene::Scene& scene)
	{
		auto& camera = scene.camera().getComponent<Component::Camera>();
		camera.aspect = m_swapChain->extentAspectRatio();

		FrameInfo frameInfo{
			m_currentFrameIndex,
			commandBuffer,
			m_globalDescriptorSets[m_currentFrameIndex],
			camera,
			&scene
		};

		updateGlobalUBO(frameInfo);

		for (auto&& [_, system] : m_renderSystems)
		{
			system->render(frameInfo);
		}
	}
	
	void Renderer::endRenderFrame(VkCommandBuffer commandBuffer)
	{
		endSwapChainRenderPass(commandBuffer);
		endFrame();
	}

	void Graphics::Renderer::shutdown()
	{
		vkDeviceWaitIdle(m_device.device());
	}

	void Renderer::createCommandBuffers()
	{
		m_commandBuffers.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = m_device.commandPool();
		allocInfo.commandBufferCount = static_cast<uint32_t>(m_commandBuffers.size());

		if (vkAllocateCommandBuffers(m_device.device(), &allocInfo, m_commandBuffers.data()) != VK_SUCCESS)
			throw std::runtime_error("failed to allocate command buffers");
	}

	void Renderer::freeCommandBuffers()
	{
		vkFreeCommandBuffers(
			m_device.device(),
			m_device.commandPool(),
			static_cast<uint32_t>(m_commandBuffers.size()),
			m_commandBuffers.data());

		m_commandBuffers.clear();
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

		// Todo
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
			throw std::runtime_error("failed to aquire swap chain image");

		m_isFrameStarted = true;

		auto commandBuffer = currentCommandBuffer();
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
			throw std::runtime_error("failed to begin recording command buffer");

		return commandBuffer;
	}

	void Renderer::endFrame()
	{
		assert(m_isFrameStarted && "Cannot call endFrame while frame is not in progress");
		auto commandBuffer = currentCommandBuffer();
		if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
			throw std::runtime_error("failed to record command buffer");

		auto result = m_swapChain->submitCommandBuffers(&commandBuffer, &m_currentImageIndex);
		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_window.wasWindowResized())
		{
			m_window.resetWindowResizedFlag();
			recreateSwapChain();
		}
		else if (result != VK_SUCCESS)
		{
			throw std::runtime_error("failed to present swap chain image");
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

	void Renderer::initializeGlobalUBO()
	{
		m_globalSetLayout = DescriptorSetLayout::Builder(m_device)
			.addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
			.build();

		m_globalUniformBuffers.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);
		m_globalDescriptorSets.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);
		for (int i = 0; i < SwapChain::MAX_FRAMES_IN_FLIGHT; i++)
		{
			m_globalUniformBuffers[i] = std::make_unique<Buffer>(m_device, sizeof(GlobalUbo), 1,
				VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
			m_globalUniformBuffers[i]->map();

			DescriptorWriter(*m_globalSetLayout, *m_globalPool)
				.writeBuffer(0, m_globalUniformBuffers[i]->descriptorInfo())
				.build(m_globalDescriptorSets[i]);
		}
	}


	void Graphics::Renderer::updateGlobalUBO(const FrameInfo& frameInfo)
	{
		GlobalUbo ubo{};
		ubo.projection = frameInfo.camera.projectionMatrix;
		ubo.view = frameInfo.camera.viewMatrix;
		ubo.inverseView = frameInfo.camera.inverseViewMatrix;

		int lighIndex = 0;
		auto view = frameInfo.scene->viewEntities<Aegix::Component::Transform, Aegix::Component::PointLight>();
		for (auto&& [entity, transform, pointLight] : view.each())
		{
			assert(lighIndex < GlobalLimits::MAX_LIGHTS && "Point lights exceed maximum number of point lights");
			ubo.pointLights[lighIndex].position = glm::vec4(transform.location, 1.0f);
			ubo.pointLights[lighIndex].color = glm::vec4(pointLight.color, pointLight.intensity);
			lighIndex++;
		}
		ubo.numLights = lighIndex;

		m_globalUniformBuffers[m_currentFrameIndex]->writeToBuffer(&ubo);
		m_globalUniformBuffers[m_currentFrameIndex]->flush();
	}
}
