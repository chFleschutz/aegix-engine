#include "vulkanite_engine.h"

#include "core/input.h"
#include "core/math_utilities.h"
#include "renderer/buffer.h"
#include "scene/components.h"
#include "scene/entity.h"
#include "systems/simple_render_system.h"
#include "systems/point_light_system.h"
#include "keyboard_movement_controller.h"

#include <array>
#include <chrono>
#include <iostream>
#include <stdexcept>

namespace vre
{
	VulkaniteEngine::VulkaniteEngine()
	{
		m_globalPool = DescriptorPool::Builder(m_device)
			.setMaxSets(SwapChain::MAX_FRAMES_IN_FLIGHT)
			.addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, SwapChain::MAX_FRAMES_IN_FLIGHT)
			.build();
	}

	VulkaniteEngine::~VulkaniteEngine()
	{
	}

	void vre::VulkaniteEngine::run()
	{
		// ****
		// Init
		std::vector<std::unique_ptr<Buffer>> uboBuffers(SwapChain::MAX_FRAMES_IN_FLIGHT);
		for (int i = 0; i < uboBuffers.size(); i++)
		{
			uboBuffers[i] = std::make_unique<Buffer>(
				m_device,
				sizeof(GlobalUbo),
				1,
				VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
			);
			uboBuffers[i]->map();
		}

		auto globalSetLayout = DescriptorSetLayout::Builder(m_device)
			.addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
			.build();

		std::vector<VkDescriptorSet> globalDescriptorSets(SwapChain::MAX_FRAMES_IN_FLIGHT);
		for (int i = 0; i < globalDescriptorSets.size(); i++)
		{
			auto bufferInfo = uboBuffers[i]->descriptorInfo();
			DescriptorWriter(*globalSetLayout, *m_globalPool)
				.writeBuffer(0, &bufferInfo)
				.build(globalDescriptorSets[i]);
		}

		SimpleRenderSystem simpleRenderSystem{ m_device, m_renderer.swapChainRenderPass(), globalSetLayout->descriptorSetLayout() };
		PointLightSystem pointLightSystem{ m_device, m_renderer.swapChainRenderPass(), globalSetLayout->descriptorSetLayout() };

		// Init Camera
		auto& camera = m_scene->camera().getComponent<CameraComponent>().Camera;
		auto& cameraTransform = m_scene->camera().getComponent<TransformComponent>();

		// Init Input
		Input::instance().initialize(m_window.glfwWindow());

		// Init Entity Components
		m_scene->runtimeBegin();

		auto currentTime = std::chrono::high_resolution_clock::now();

		// ***********
		// update loop
		while (!m_window.shouldClose())
		{
			glfwPollEvents();

			// Calculate time
			auto newTime = std::chrono::high_resolution_clock::now();
			float frameTimeSec = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
			currentTime = newTime;

			// Update all components
			m_scene->update(frameTimeSec);

			float aspect = m_renderer.aspectRatio();
			camera.setPerspectiveProjection(glm::radians(50.0f), aspect, 0.1f, 100.0f);
			camera.setViewYXZ(cameraTransform.Location, cameraTransform.Rotation);

			// RENDERING
			if (auto commandBuffer = m_renderer.beginFrame())
			{
				int frameIndex = m_renderer.frameIndex();
				FrameInfo frameInfo{
					frameIndex,
					frameTimeSec,
					commandBuffer,
					&camera,
					globalDescriptorSets[frameIndex],
					m_scene.get()
				};

				// update
				GlobalUbo ubo{};
				ubo.projection = camera.projectionMatrix();
				ubo.view = camera.viewMatrix();
				ubo.inverseView = camera.inverseViewMatrix();

				pointLightSystem.update(frameInfo, ubo);

				uboBuffers[frameIndex]->writeToBuffer(&ubo);
				uboBuffers[frameIndex]->flush();

				// render
				m_renderer.beginSwapChainRenderPass(commandBuffer);

				// render solid objects first
				simpleRenderSystem.renderGameObjects(frameInfo);
				pointLightSystem.render(frameInfo);

				m_renderer.endSwapChainRenderPass(commandBuffer);
				m_renderer.endFrame();
			}
		}
		vkDeviceWaitIdle(m_device.device());

		m_scene->runtimeEnd();
	}

} // namespace vre
