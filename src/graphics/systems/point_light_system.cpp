#include "point_light_system.h"

#include "graphics/camera.h"
#include "scene/components.h"
#include "utils/math_utils.h"

#include <array>
#include <map>
#include <stdexcept>

namespace Aegix::Graphics
{
	PointLightSystem::PointLightSystem(VulkanDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout)
		: RenderSystem(device)
	{
		std::vector<VkDescriptorSetLayout> descriptorSetLayouts{ globalSetLayout };
		createPipelineLayout(descriptorSetLayouts, sizeof(PointLightPushConstants));

		m_pipeline = Pipeline::Builder(m_device)
			.setRenderPass(renderPass)
			.setPipelineLayout(m_pipelineLayout)
			.addShaderStage(VK_SHADER_STAGE_VERTEX_BIT, SHADER_DIR "point_light.vert.spv")
			.addShaderStage(VK_SHADER_STAGE_FRAGMENT_BIT, SHADER_DIR "point_light.frag.spv")
			.build();
	}

	void PointLightSystem::render(const FrameInfo& frameInfo)
	{
		m_pipeline->bind(frameInfo.commandBuffer);

		vkCmdBindDescriptorSets(
			frameInfo.commandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			m_pipelineLayout,
			0, 1,
			&frameInfo.globalDescriptorSet,
			0, nullptr
		);

		auto view = frameInfo.scene->viewEntitiesByType<Aegix::Component::Transform, Aegix::Component::PointLight>();
		view.use<Aegix::Component::Transform>(); // Sort because of Transparency
		for (auto&& [entity, transform, pointLight] : view.each())
		{
			PointLightPushConstants push{};
			push.position = Vector4(transform.location, 1.0f);
			push.color = Vector4(pointLight.color.rgb(), 1.0f);
			push.radius = transform.scale.x;

			vkCmdPushConstants(
				frameInfo.commandBuffer,
				m_pipelineLayout,
				VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
				0,
				sizeof(PointLightPushConstants),
				&push);

			vkCmdDraw(frameInfo.commandBuffer, 6, 1, 0, 0);
		}
	}

	//void PointLightSystem::createPipelineLayout(VkDescriptorSetLayout globalSetLayout)
	//{
	//	VkPushConstantRange pushConstantRange{};
	//	pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
	//	pushConstantRange.offset = 0;
	//	pushConstantRange.size = sizeof(PointLightPushConstants);

	//	std::vector<VkDescriptorSetLayout> descriptorSetLayouts{ globalSetLayout };

	//	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	//	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	//	pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
	//	pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
	//	pipelineLayoutInfo.pushConstantRangeCount = 1;
	//	pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

	//	if (vkCreatePipelineLayout(m_device.device(), &pipelineLayoutInfo, nullptr, &mPipelineLayout) != VK_SUCCESS)
	//		throw std::runtime_error("failed to create pipeline layout");
	//}

	//void PointLightSystem::createPipeline(VkRenderPass renderPass)
	//{
	//	assert(mPipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

	//	PipelineConfigInfo pipelineConfig{};
	//	Pipeline::defaultPipelineConfigInfo(pipelineConfig);
	//	Pipeline::enableAlphaBlending(pipelineConfig);
	//	pipelineConfig.bindingDescriptions.clear();
	//	pipelineConfig.attributeDescriptions.clear();
	//	pipelineConfig.renderPass = renderPass;
	//	pipelineConfig.pipelineLayout = mPipelineLayout;

	//	std::string vertShaderPath = SHADER_DIR "point_light.vert.spv";
	//	std::string fragShaderPath = SHADER_DIR "point_light.frag.spv";

	//	mPipeline = std::make_unique<Pipeline>(
	//		m_device,
	//		vertShaderPath,
	//		fragShaderPath,
	//		pipelineConfig);
	//}
}
