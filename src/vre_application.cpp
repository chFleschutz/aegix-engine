#include "vre_application.h"

#include "keyboard_movement_controller.h"
#include "vre_buffer.h"
#include "vre_camera.h"
#include "simple_render_system.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <array>
#include <chrono>
#include <stdexcept>

namespace vre
{
	struct GlobalUbo
	{
		glm::mat4 projectionView{1.0f};
		glm::vec4 ambientLightColor{1.0f, 1.0f, 1.0f, 0.02f}; // w is the intesity
		glm::vec3 lightPosition{-0.5f, -1.0f, -1.0f};
		alignas(16) glm::vec4 lightColor{1.0f}; // w is the light intensity
	};

	VreApplication::VreApplication()
	{
		mGlobalPool = VreDescriptorPool::Builder(mVreDevice)
			.setMaxSets(VreSwapChain::MAX_FRAMES_IN_FLIGHT)
			.addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VreSwapChain::MAX_FRAMES_IN_FLIGHT)
			.build();

		loadGameObjects();
	}

	VreApplication::~VreApplication()
	{
	}

	void vre::VreApplication::run()
	{
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

		VreCamera camera{};
        auto viewerObject = VreGameObject::createGameObject();
		viewerObject.transform.translation = { 1.0f, -0.5f, -2.0f };
		viewerObject.transform.rotation = { -0.2f, 5.86f, 0.0f };
        KeyboardMovementController cameraController{};

        auto currentTime = std::chrono::high_resolution_clock::now();

		while (!mVreWindow.shouldClose())
		{
			glfwPollEvents();

            auto newTime = std::chrono::high_resolution_clock::now();
            float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
            currentTime = newTime;

            cameraController.moveInPlaneXZ(mVreWindow.glfwWindow(), frameTime, viewerObject);
            camera.setViewYXZ(viewerObject.transform.translation, viewerObject.transform.rotation);

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
					mGameObjects
				};

				// update
				GlobalUbo ubo{};
				ubo.projectionView = camera.projectionMatrix() * camera.viewMatrix();
				uboBuffers[frameIndex]->writeToBuffer(&ubo);
				uboBuffers[frameIndex]->flush();

				// render
				mVreRenderer.beginSwapChainRenderPass(commandBuffer);
				simpleRenderSystem.renderGameObjects(frameInfo);
				mVreRenderer.endSwapChainRenderPass(commandBuffer);
				mVreRenderer.endFrame();
			}
		}

		vkDeviceWaitIdle(mVreDevice.device());
	}

	void VreApplication::loadGameObjects()
	{
		std::shared_ptr<VreModel> vreModel = VreModel::createModelFromFile(mVreDevice, "data/models/flat_vase.obj");
        auto flatVase = VreGameObject::createGameObject();
		flatVase.model = vreModel;
		flatVase.transform.translation = { -0.75f, 0.5f, 0.0f };
		flatVase.transform.scale = glm::vec3{ 3.0f };
        mGameObjects.emplace(flatVase.id(), std::move(flatVase));

		vreModel = VreModel::createModelFromFile(mVreDevice, "data/models/smooth_vase.obj");
		auto smoothVase = VreGameObject::createGameObject();
		smoothVase.model = vreModel;
		smoothVase.transform.translation = { 0.75f, 0.5f, 0.0f };
		smoothVase.transform.scale = glm::vec3{ 3.0f };
		mGameObjects.emplace(smoothVase.id(), std::move(smoothVase));

		vreModel = VreModel::createModelFromFile(mVreDevice, "data/models/plane.obj");
		auto floor = VreGameObject::createGameObject();
		floor.model = vreModel;
		floor.transform.translation = { 0.0f, 0.5f, 0.0f };
		floor.transform.scale = glm::vec3{ 3.0f };
		mGameObjects.emplace(floor.id(), std::move(floor));
	}

} // namespace vre
