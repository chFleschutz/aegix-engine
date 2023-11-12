#include "vulkanite_engine.h"

#include "core/input.h"
#include "graphics/buffer.h"
#include "graphics/systems/point_light_system.h"
#include "graphics/systems/simple_render_system.h"
#include "scene/components.h"
#include "scene/entity.h"
#include "scripting/script_base.h"
#include "utils/math_utils.h"

#include <array>
#include <chrono>
#include <iostream>
#include <stdexcept>

namespace Vulkanite
{
	Engine::Engine()
	{
		m_globalPool = VEGraphics::DescriptorPool::Builder(m_device)
			.setMaxSets(VEGraphics::SwapChain::MAX_FRAMES_IN_FLIGHT)
			.addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VEGraphics::SwapChain::MAX_FRAMES_IN_FLIGHT)
			.build();
	}

	Engine::~Engine()
	{
	}

	void Engine::run()
	{
		// ****
		// Init
		std::vector<std::unique_ptr<VEGraphics::Buffer>> uboBuffers(VEGraphics::SwapChain::MAX_FRAMES_IN_FLIGHT);
		for (int i = 0; i < uboBuffers.size(); i++)
		{
			uboBuffers[i] = std::make_unique<VEGraphics::Buffer>(
				m_device,
				sizeof(VEGraphics::GlobalUbo),
				1,
				VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
			);
			uboBuffers[i]->map();
		}

		auto globalSetLayout = VEGraphics::DescriptorSetLayout::Builder(m_device)
			.addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
			.build();

		std::vector<VkDescriptorSet> globalDescriptorSets(VEGraphics::SwapChain::MAX_FRAMES_IN_FLIGHT);
		for (int i = 0; i < globalDescriptorSets.size(); i++)
		{
			auto bufferInfo = uboBuffers[i]->descriptorInfo();
			VEGraphics::DescriptorWriter(*globalSetLayout, *m_globalPool)
				.writeBuffer(0, &bufferInfo)
				.build(globalDescriptorSets[i]);
		}

		VEGraphics::SimpleRenderSystem simpleRenderSystem{ m_device, m_renderer.swapChainRenderPass(), globalSetLayout->descriptorSetLayout() };
		VEGraphics::PointLightSystem pointLightSystem{ m_device, m_renderer.swapChainRenderPass(), globalSetLayout->descriptorSetLayout() };

		// Init Input
		Input::instance().initialize(m_window.glfwWindow());

		// Init Scene
		m_scene->initialize();

		// Init Camera
		auto& camera = m_scene->camera().getComponent<VEComponent::Camera>().camera;
		auto& cameraTransform = m_scene->camera().getComponent<VEComponent::Transform>();

		auto currentTime = std::chrono::high_resolution_clock::now();

		// ***********
		// update loop
		while (!m_window.shouldClose())
		{
			// Calculate time
			auto frameBeginTime = std::chrono::high_resolution_clock::now();
			float frameTimeSec = std::chrono::duration<float, std::chrono::seconds::period>(frameBeginTime - currentTime).count();
			currentTime = frameBeginTime;

			glfwPollEvents();

			// Update all components
			m_scene->update(frameTimeSec);

			float aspect = m_renderer.aspectRatio();
			camera.setPerspectiveProjection(glm::radians(50.0f), aspect, 0.1f, 100.0f);
			camera.setViewYXZ(cameraTransform.location, cameraTransform.rotation);

			// RENDERING
			if (auto commandBuffer = m_renderer.beginFrame())
			{
				int frameIndex = m_renderer.frameIndex();
				VEGraphics::FrameInfo frameInfo{
					frameIndex,
					frameTimeSec,
					commandBuffer,
					&camera,
					globalDescriptorSets[frameIndex],
					m_scene.get()
				};

				// update
				VEGraphics::GlobalUbo ubo{};
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

			applyFrameBrake(frameBeginTime);
		}
		vkDeviceWaitIdle(m_device.device());

		m_scene->runtimeEnd();
	}

	void Engine::applyFrameBrake(std::chrono::steady_clock::time_point frameBeginTime)
	{
		if (MAX_FPS <= 0)
			return;

		auto desiredFrameTime = std::chrono::round<std::chrono::nanoseconds>(std::chrono::duration<float>(1.0f / MAX_FPS));
		while (std::chrono::high_resolution_clock::now() < frameBeginTime + desiredFrameTime);
	}

} // namespace vre
