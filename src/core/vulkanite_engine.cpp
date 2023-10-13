#include "vulkanite_engine.h"

#include "renderer/buffer.h"
#include "scene/components.h"
#include "scene/entity.h"
#include "core/input.h"
#include "keyboard_movement_controller.h"
#include "systems/simple_render_system.h"
#include "systems/point_light_system.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <array>
#include <chrono>
#include <iostream>
#include <stdexcept>


namespace vre
{
	VulkaniteEngine::VulkaniteEngine()
	{
		mGlobalPool = DescriptorPool::Builder(mDevice)
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
		std::vector<std::unique_ptr<VreBuffer>> uboBuffers(SwapChain::MAX_FRAMES_IN_FLIGHT);
		for (int i = 0; i < uboBuffers.size(); i++)
		{
			uboBuffers[i] = std::make_unique<VreBuffer>(
				mDevice,
				sizeof(GlobalUbo),
				1,
				VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
			);
			uboBuffers[i]->map();
		}

		auto globalSetLayout = DescriptorSetLayout::Builder(mDevice)
			.addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
			.build();

		std::vector<VkDescriptorSet> globalDescriptorSets(SwapChain::MAX_FRAMES_IN_FLIGHT);
		for (int i = 0; i < globalDescriptorSets.size(); i++)
		{
			auto bufferInfo = uboBuffers[i]->descriptorInfo();
			DescriptorWriter(*globalSetLayout, *mGlobalPool)
				.writeBuffer(0, &bufferInfo)
				.build(globalDescriptorSets[i]);
		}

		SimpleRenderSystem simpleRenderSystem{ mDevice, mRenderer.swapChainRenderPass(), globalSetLayout->descriptorSetLayout() };
		PointLightSystem pointLightSystem{ mDevice, mRenderer.swapChainRenderPass(), globalSetLayout->descriptorSetLayout() };

		// Init Camera
		auto& camera = mScene->camera().getComponent<CameraComponent>().Camera;
		auto& cameraTransform = mScene->camera().getComponent<TransformComponent>();

		// Init Input
		Input::instance().initialize(mWindow.glfwWindow());

		// Init Entity Components
		mScene->runtimeBegin();

		auto currentTime = std::chrono::high_resolution_clock::now();

		// ***********
		// update loop
		while (!mWindow.shouldClose())
		{
			glfwPollEvents();

			// Calculate time
			auto newTime = std::chrono::high_resolution_clock::now();
			float frameTimeSec = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
			currentTime = newTime;

			// Update all components
			mScene->update(frameTimeSec);

			float aspect = mRenderer.aspectRatio();
			camera.setPerspectiveProjection(glm::radians(50.0f), aspect, 0.1f, 100.0f);
			camera.setViewYXZ(cameraTransform.Location, cameraTransform.Rotation);

			// RENDERING
			if (auto commandBuffer = mRenderer.beginFrame())
			{
				int frameIndex = mRenderer.frameIndex();
				FrameInfo frameInfo{
					frameIndex,
					frameTimeSec,
					commandBuffer,
					&camera,
					globalDescriptorSets[frameIndex],
					mScene.get()
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
				mRenderer.beginSwapChainRenderPass(commandBuffer);

				// render solid objects first
				simpleRenderSystem.renderGameObjects(frameInfo);
				pointLightSystem.render(frameInfo);

				mRenderer.endSwapChainRenderPass(commandBuffer);
				mRenderer.endFrame();
			}
		}
		vkDeviceWaitIdle(mDevice.device());

		mScene->runtimeEnd();
	}

} // namespace vre
