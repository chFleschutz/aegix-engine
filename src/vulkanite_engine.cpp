#include "vulkanite_engine.h"

#include "buffer.h"
#include "scene/components.h"
#include "scene/entity.h"
#include "input.h"
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

		// Init Camera
		auto& camera = mScene->camera().getComponent<CameraComponent>().Camera;

		// Init Input
		Input::instance().initialize(mVreWindow.glfwWindow());

		// Init Entity Components
		initializeComponents();

		auto currentTime = std::chrono::high_resolution_clock::now();

		// ***********
		// update loop
		while (!mVreWindow.shouldClose())
		{
			glfwPollEvents();

			// Calculate time
			auto newTime = std::chrono::high_resolution_clock::now();
			float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
			currentTime = newTime;

			// Update all components
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
		//for (auto& [id, entity] : mScene->entities())
		//{
		//	for (auto& component : entity.components())
		//	{
		//		component->begin();
		//	}
		//}
		std::cout << "Components initialized" << std::endl;
	}

	void VulkaniteEngine::updateComponets(float deltaSeconds)
	{
		//for (auto& [id, entity] : mScene->entities())
		//{
		//	for (auto& component : entity.components())
		//	{
		//		component->update(deltaSeconds);
		//	}
		//}
	}

	void VulkaniteEngine::cleanupComponents()
	{
		//for (auto& [id, entity] : mScene->entities())
		//{
		//	for (auto& component : entity.components())
		//	{
		//		component->end();
		//	}
		//}
		std::cout << "Components cleaned up" << std::endl;
	}

} // namespace vre
