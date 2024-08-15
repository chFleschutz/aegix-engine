#include "point_light_system.h"

#include "scene/components.h"
#include "utils/math_utils.h"

namespace Aegix::Graphics
{
	PointLightSystem::PointLightSystem(VulkanDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout)
		: RenderSystem(device)
	{
		m_pipelineLayout = PipelineLayout::Builder(m_device)
			.addDescriptorSetLayout(globalSetLayout)
			.addPushConstantRange(VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(PointLightPushConstants))
			.build();

		m_pipeline = Pipeline::Builder(m_device)
			.setRenderPass(renderPass)
			.setPipelineLayout(m_pipelineLayout->pipelineLayout())
			.addShaderStage(VK_SHADER_STAGE_VERTEX_BIT, SHADER_DIR "point_light.vert.spv")
			.addShaderStage(VK_SHADER_STAGE_FRAGMENT_BIT, SHADER_DIR "point_light.frag.spv")
			.setVertexBindingDescriptions({}) // Clear default vertex binding
			.setVertexAttributeDescriptions({}) // Clear default vertex attributes
			.enableAlphaBlending()
			.build();
	}

	void PointLightSystem::render(const FrameInfo& frameInfo)
	{
		const float pointLightScale = 0.01f;

		m_pipeline->bind(frameInfo.commandBuffer);

		vkCmdBindDescriptorSets(
			frameInfo.commandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			m_pipelineLayout->pipelineLayout(),
			0, 1,
			&frameInfo.globalDescriptorSet,
			0, nullptr
		);

		auto view = frameInfo.scene->viewEntities<Aegix::Component::Transform, Aegix::Component::PointLight>();
		view.use<Aegix::Component::Transform>(); // Sort because of Transparency
		for (auto&& [entity, transform, pointLight] : view.each())
		{
			PointLightPushConstants push{};
			push.position = Vector4(transform.location, 1.0f);
			push.color = Vector4(pointLight.color.rgb(), 1.0f);
			push.radius = pointLight.intensity * pointLightScale;

			vkCmdPushConstants(
				frameInfo.commandBuffer,
				m_pipelineLayout->pipelineLayout(),
				VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
				0,
				sizeof(PointLightPushConstants),
				&push
			);

			vkCmdDraw(frameInfo.commandBuffer, 6, 1, 0, 0);
		}
	}
}
