#include "point_light_system.h"

#include "graphics/vulkan_tools.h"
#include "scene/components.h"
#include "utils/math_utils.h"

namespace Aegix::Graphics
{
	PointLightSystem::PointLightSystem(VulkanDevice& device, VkDescriptorSetLayout globalSetLayout)
		: RenderSystem(device, globalSetLayout)
	{
		m_pipelineLayout = PipelineLayout::Builder(m_device)
			.addDescriptorSetLayout(globalSetLayout)
			.addPushConstantRange(VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(PointLightPushConstants))
			.build();

		m_pipeline = Pipeline::GraphicsBuilder(m_device, *m_pipelineLayout)
			.addShaderStage(VK_SHADER_STAGE_VERTEX_BIT, SHADER_DIR "point_light.vert.spv")
			.addShaderStage(VK_SHADER_STAGE_FRAGMENT_BIT, SHADER_DIR "point_light.frag.spv")
			.addColorAttachment(VK_FORMAT_R16G16B16A16_SFLOAT, true)
			.setDepthAttachment(VK_FORMAT_D32_SFLOAT)
			.setVertexBindingDescriptions({}) // Clear default vertex binding
			.setVertexAttributeDescriptions({}) // Clear default vertex attributes
			.build();
	}

	void PointLightSystem::render(const FrameInfo& frameInfo, VkDescriptorSet globalSet)
	{
		constexpr float pointLightScale = 0.005f;

		VkCommandBuffer cmd = frameInfo.commandBuffer;

		m_pipeline->bind(cmd);
		Tools::vk::cmdBindDescriptorSet(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, *m_pipelineLayout, globalSet);

		auto view = frameInfo.scene.viewEntities<Aegix::Component::Transform, Aegix::Component::PointLight>();
		for (auto&& [entity, transform, pointLight] : view.each())
		{
			PointLightPushConstants push{
				.position = glm::vec4(transform.location, 1.0f),
				.color = glm::vec4(pointLight.color, 1.0f),
				.radius = pointLight.intensity * pointLightScale * transform.scale.x
			};

			vkCmdPushConstants(cmd,
				*m_pipelineLayout,
				VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
				0,
				sizeof(PointLightPushConstants),
				&push
			);

			vkCmdDraw(cmd, 6, 1, 0, 0);
		}
	}
}
