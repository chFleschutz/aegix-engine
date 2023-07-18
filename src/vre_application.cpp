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
		alignas(16) glm::mat4 projectionView{1.0f};
		alignas(16) glm::vec3 lightDirection = glm::normalize(glm::vec3{1.0f, -3.0f, -1.0f});
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
			.addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
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
        camera.setViewTarget(glm::vec3(-1.0f, -2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 2.5f));

        auto viewerObject = VreGameObject::createGameObject();
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
            camera.setPerspectiveProjection(glm::radians(50.0f), aspect, 0.1f, 10.0f);

			if (auto commandBuffer = mVreRenderer.beginFrame())
			{
				int frameIndex = mVreRenderer.frameIndex();
				FrameInfo frameInfo{
					frameIndex,
					frameTime,
					commandBuffer,
					camera,
					globalDescriptorSets[frameIndex]
				};

				// update
				GlobalUbo ubo{};
				ubo.projectionView = camera.projectionMatrix() * camera.viewMatrix();
				uboBuffers[frameIndex]->writeToBuffer(&ubo);
				uboBuffers[frameIndex]->flush();

				// render
				mVreRenderer.beginSwapChainRenderPass(commandBuffer);
				simpleRenderSystem.renderGameObjects(frameInfo, mGameObjects);
				mVreRenderer.endSwapChainRenderPass(commandBuffer);
				mVreRenderer.endFrame();
			}
		}

		vkDeviceWaitIdle(mVreDevice.device());
	}

	void VreApplication::loadGameObjects()
	{
		std::shared_ptr<VreModel> vreModel = VreModel::createModelFromFile(mVreDevice, "data/models/flat_vase.obj");

        auto gameObject = VreGameObject::createGameObject();
        gameObject.model = vreModel;
        gameObject.transform.translation = { 0.0f, 0.5f, 2.5f };
		gameObject.transform.scale = glm::vec3{ 3.0f };
        mGameObjects.push_back(std::move(gameObject));
	}

} // namespace vre
