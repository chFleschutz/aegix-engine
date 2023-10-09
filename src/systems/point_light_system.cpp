#include "point_light_system.h"

#include "./../scene/components.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <array>
#include <map>
#include <stdexcept>

namespace vre
{
	struct PointLightPushConstants
	{
		glm::vec4 position{};
		glm::vec4 color{};
		float radius;
	};

	PointLightSystem::PointLightSystem(VreDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout) : mVreDevice{ device }
	{
		createPipelineLayout(globalSetLayout);
		createPipeline(renderPass);
	}

	PointLightSystem::~PointLightSystem()
	{
		vkDestroyPipelineLayout(mVreDevice.device(), mPipelineLayout, nullptr);
	}

	void PointLightSystem::update(FrameInfo& frameInfo, GlobalUbo& ubo)
	{
		int lighIndex = 0;
		for (auto&& [entity, transform, pointLight] : frameInfo.scene->viewEntitiesByType<TransformComponent, PointLightComponent>().each())
		{
			assert(lighIndex < MAX_LIGHTS && "Point lights exceed maximum number of point lights");
			ubo.pointLights[lighIndex].position = glm::vec4(transform.Location, 1.0f);
			ubo.pointLights[lighIndex].color = glm::vec4(pointLight.Color, pointLight.Intensity);
			lighIndex++;
		}
		ubo.numLights = lighIndex;
	}

	void PointLightSystem::render(FrameInfo& frameInfo)
	{
		//// sort lights because of transparency
		//std::map<float, Entity> sortedLights;
		//for (auto& objPair : frameInfo.gameObjects)
		//{
		//	auto& obj = objPair.second;
		//	if (obj.pointLight == nullptr) 
		//		continue;

		//	auto offset = frameInfo.camera->position() - obj.transform.location;
		//	float disSquared = glm::dot(offset, offset);
		//	sortedLights[disSquared] = obj.id();
		//}

		mVrePipeline->bind(frameInfo.commandBuffer);

		vkCmdBindDescriptorSets(
			frameInfo.commandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			mPipelineLayout,
			0, 1,
			&frameInfo.globalDescriptorSet,
			0, nullptr
		);

		auto view = frameInfo.scene->viewEntitiesByType<TransformComponent, PointLightComponent>();
		view.use<TransformComponent>(); // Sort because of Transparency
		for (auto&& [entity, transform, pointLight] : view.each())
		{
			PointLightPushConstants push{};
			push.position = glm::vec4(transform.Location, 1.0f);
			push.color = glm::vec4(pointLight.Color, 1.0f);
			push.radius = transform.Scale.x;

			vkCmdPushConstants(
				frameInfo.commandBuffer,
				mPipelineLayout,
				VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
				0,
				sizeof(PointLightPushConstants),
				&push);
			vkCmdDraw(frameInfo.commandBuffer, 6, 1, 0, 0);
		}

		//for (auto it = sortedLights.rbegin(); it != sortedLights.rend(); ++it)
		//{
		//	auto& obj = frameInfo.gameObjects.at(it->second);

		//	PointLightPushConstants push{};
		//	push.position = glm::vec4(obj.transform.location, 1.0f);
		//	push.color = glm::vec4(obj.color, 1.0f);
		//	push.radius = obj.transform.scale.x;

		//	vkCmdPushConstants(
		//		frameInfo.commandBuffer,
		//		mPipelineLayout,
		//		VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
		//		0,
		//		sizeof(PointLightPushConstants),
		//		&push);
		//	vkCmdDraw(frameInfo.commandBuffer, 6, 1, 0, 0);
		//}
	}

	void PointLightSystem::createPipelineLayout(VkDescriptorSetLayout globalSetLayout)
	{
		VkPushConstantRange pushConstantRange{};
		pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		pushConstantRange.offset = 0;
		pushConstantRange.size = sizeof(PointLightPushConstants);

		std::vector<VkDescriptorSetLayout> descriptorSetLayouts{globalSetLayout};

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
		pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
		pipelineLayoutInfo.pushConstantRangeCount = 1;
		pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

		if (vkCreatePipelineLayout(mVreDevice.device(), &pipelineLayoutInfo, nullptr, &mPipelineLayout) != VK_SUCCESS)
			throw std::runtime_error("failed to create pipeline layout");
	}

	void PointLightSystem::createPipeline(VkRenderPass renderPass)
	{
		assert(mPipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

		PipelineConfigInfo pipelineConfig{};
		VrePipeline::defaultPipelineConfigInfo(pipelineConfig);
		VrePipeline::enableAlphaBlending(pipelineConfig);
		pipelineConfig.bindingDescriptions.clear();
		pipelineConfig.attributeDescriptions.clear();
		pipelineConfig.renderPass = renderPass;
		pipelineConfig.pipelineLayout = mPipelineLayout;

		mVrePipeline = std::make_unique<VrePipeline>(
			mVreDevice,
			"shaders/point_light.vert.spv",
			"shaders/point_light.frag.spv",
			pipelineConfig);
	}

} // namespace vre
