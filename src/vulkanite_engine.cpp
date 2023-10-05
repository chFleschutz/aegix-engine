#include "vulkanite_engine.h"

#include "keyboard_movement_controller.h"
#include "buffer.h"
#include "camera.h"
#include "systems/simple_render_system.h"
#include "systems/point_light_system.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <array>
#include <chrono>
#include <stdexcept>

#include <iostream>

namespace vre
{
	VulkaniteEngine::VulkaniteEngine()
	{
		mGlobalPool = VreDescriptorPool::Builder(mVreDevice)
			.setMaxSets(VreSwapChain::MAX_FRAMES_IN_FLIGHT)
			.addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VreSwapChain::MAX_FRAMES_IN_FLIGHT)
			.build();
	}

	VulkaniteEngine::~VulkaniteEngine()
	{
	}

	void vre::VulkaniteEngine::run()
	{
		// ****
		// Init
		std::vector<std::unique_ptr<VreBuffer>> uboBuffers(VreSwapChain::MAX_FRAMES_IN_FLIGHT);
		for (int i = 0; i < uboBuffers.size(); i++)
		{
			uboBuffers[i] = std::make_unique<VreBuffer>(
				mVreDevice,
				sizeof(GlobalUbo),
				1,
				VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
			);
			uboBuffers[i]->map();
		}

		auto globalSetLayout = VreDescriptorSetLayout::Builder(mVreDevice)
			.addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
			.build();

		std::vector<VkDescriptorSet> globalDescriptorSets(VreSwapChain::MAX_FRAMES_IN_FLIGHT);
		for (int i = 0; i < globalDescriptorSets.size(); i++)
		{
			auto bufferInfo = uboBuffers[i]->descriptorInfo();
			VreDescriptorWriter(*globalSetLayout, *mGlobalPool)
				.writeBuffer(0, &bufferInfo)
				.build(globalDescriptorSets[i]);
		}

		SimpleRenderSystem simpleRenderSystem{ mVreDevice, mVreRenderer.swapChainRenderPass(), globalSetLayout->descriptorSetLayout() };
		PointLightSystem pointLightSystem{ mVreDevice, mVreRenderer.swapChainRenderPass(), globalSetLayout->descriptorSetLayout() };

		VreCamera camera{};
		auto viewerObject = SceneEntity::createEmpty();
		viewerObject.transform.location = { 1.0f, -0.5f, -2.0f };
		viewerObject.transform.rotation = { -0.2f, 5.86f, 0.0f };
		KeyboardMovementController cameraController{ mVreWindow.glfwWindow() };

		auto currentTime = std::chrono::high_resolution_clock::now();

		initializeComponents();

		// ***********
		// update loop
		while (!mVreWindow.shouldClose())
		{
			glfwPollEvents();

			auto newTime = std::chrono::high_resolution_clock::now();
			float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
			currentTime = newTime;

			cameraController.applyInput(mVreWindow.glfwWindow(), frameTime, viewerObject);
			camera.setViewYXZ(viewerObject.transform.location, viewerObject.transform.rotation);

			updateComponets(frameTime);


			// RENDERING
			float aspect = mVreRenderer.aspectRatio();
			camera.setPerspectiveProjection(glm::radians(50.0f), aspect, 0.1f, 100.0f);

			if (auto commandBuffer = mVreRenderer.beginFrame())
			{
				int frameIndex = mVreRenderer.frameIndex();
				FrameInfo frameInfo{
					frameIndex,
					frameTime,
					commandBuffer,
					camera,
					globalDescriptorSets[frameIndex],
					mScene->entities()
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
				mVreRenderer.beginSwapChainRenderPass(commandBuffer);

				// render solid objects first
				simpleRenderSystem.renderGameObjects(frameInfo);
				pointLightSystem.render(frameInfo);

				mVreRenderer.endSwapChainRenderPass(commandBuffer);
				mVreRenderer.endFrame();
			}
		}
		vkDeviceWaitIdle(mVreDevice.device());
		cleanupComponents();
	}

	void VulkaniteEngine::initializeComponents()
	{
		for (auto& [id, entity] : mScene->entities())
		{
			for (auto& component : entity.components())
			{
				component->begin();
			}
		}
		std::cout << "Components initialized" << std::endl;
	}

	void VulkaniteEngine::updateComponets(float deltaSeconds)
	{
		for (auto& [id, entity] : mScene->entities())
		{
			for (auto& component : entity.components())
			{
				component->update(deltaSeconds);
			}
		}
	}

	void VulkaniteEngine::cleanupComponents()
	{
		for (auto& [id, entity] : mScene->entities())
		{
			for (auto& component : entity.components())
			{
				component->end();
			}
		}
		std::cout << "Components cleaned up" << std::endl;
	}

} // namespace vre
