#include "point_light_system.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <array>
#include <stdexcept>

namespace vre
{
	PointLightSystem::PointLightSystem(VreDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout) : mVreDevice{device}
	{
		createPipelineLayout(globalSetLayout);
		createPipeline(renderPass);
	}

	PointLightSystem::~PointLightSystem()
	{
		vkDestroyPipelineLayout(mVreDevice.device(), mPipelineLayout, nullptr);
	}

	void PointLightSystem::render(FrameInfo& frameInfo)
	{
		mVrePipeline->bind(frameInfo.commandBuffer);

		vkCmdBindDescriptorSets(
			frameInfo.commandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			mPipelineLayout,
			0, 1,
			&frameInfo.globalDescriptorSet,
			0, nullptr
		);

		vkCmdDraw(frameInfo.commandBuffer, 6, 1, 0, 0);
	}

	void PointLightSystem::createPipelineLayout(VkDescriptorSetLayout globalSetLayout)
	{
		//VkPushConstantRange pushConstantRange{};
		//pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		//pushConstantRange.offset = 0;
		//pushConstantRange.size = sizeof(SimplePushConstantData);

		std::vector<VkDescriptorSetLayout> descriptorSetLayouts{globalSetLayout};

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
		pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
		pipelineLayoutInfo.pushConstantRangeCount = 0;
		pipelineLayoutInfo.pPushConstantRanges = nullptr;

		if (vkCreatePipelineLayout(mVreDevice.device(), &pipelineLayoutInfo, nullptr, &mPipelineLayout) != VK_SUCCESS)
			throw std::runtime_error("failed to create pipeline layout");
	}

	void PointLightSystem::createPipeline(VkRenderPass renderPass)
	{
		assert(mPipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

		PipelineConfigInfo pipelineConfig{};
		VrePipeline::defaultPipelineConfigInfo(pipelineConfig);
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
