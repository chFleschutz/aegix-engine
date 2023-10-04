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

namespace vre
{
	VulkaniteEngine::VulkaniteEngine()
	{
		mGlobalPool = VreDescriptorPool::Builder(mVreDevice)
			.setMaxSets(VreSwapChain::MAX_FRAMES_IN_FLIGHT)
			.addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VreSwapChain::MAX_FRAMES_IN_FLIGHT)
			.build();

		loadSceneOld();
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
        auto viewerObject = VreSceneObject::createEmpty();
		viewerObject.transform.location = { 1.0f, -0.5f, -2.0f };
		viewerObject.transform.rotation = { -0.2f, 5.86f, 0.0f };
        KeyboardMovementController cameraController{ mVreWindow.glfwWindow() };

        auto currentTime = std::chrono::high_resolution_clock::now();

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
					mScene->objects()
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
	}

	void VulkaniteEngine::loadSceneOld()
	{
		{
			std::shared_ptr<VreModel> vreModel = VreModel::createModelFromFile(mVreDevice, "models/teapot.obj");
			auto flatVase = VreSceneObject::createModel(vreModel);
			flatVase.transform.location = { -0.75f, 0.5f, 0.0f };
			flatVase.transform.scale = glm::vec3{ 3.0f };
			mGameObjects.emplace(flatVase.id(), std::move(flatVase));

			vreModel = VreModel::createModelFromFile(mVreDevice, "models/plane.obj");
			auto floor = VreSceneObject::createModel(vreModel);
			floor.transform.location = { 0.0f, 0.5f, 0.0f };
			floor.transform.scale = glm::vec3{ 3.0f };
			mGameObjects.emplace(floor.id(), std::move(floor));
		}
		{
			auto pointLight = VreSceneObject::createPointLight(0.2f);
			pointLight.transform.location = { -1.0f, -1.0f, -1.0f };
			mGameObjects.emplace(pointLight.id(), std::move(pointLight));

			pointLight = VreSceneObject::createPointLight(0.2f);
			pointLight.transform.location = { 0.0f, -1.0f, -1.0f };
			mGameObjects.emplace(pointLight.id(), std::move(pointLight));
		}
	}

} // namespace vre
