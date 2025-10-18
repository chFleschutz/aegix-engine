#include "pch.h"
#include "point_light_render_system.h"

#include "core/globals.h"
#include "graphics/descriptors.h"
#include "scene/components.h"

namespace Aegix::Graphics
{
	PointLightRenderSystem::PointLightRenderSystem()
	{
		auto globalSetLayout = DescriptorSetLayout::Builder{}
			.addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS | 
				VK_SHADER_STAGE_TASK_BIT_EXT | VK_SHADER_STAGE_MESH_BIT_EXT)
			.build();

		m_pipeline = Pipeline::GraphicsBuilder{}
			.addDescriptorSetLayout(globalSetLayout)
			.addPushConstantRange(VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(PointLightPushConstants))
			.addShaderStage(VK_SHADER_STAGE_VERTEX_BIT, SHADER_DIR "point_light.vert.spv")
			.addShaderStage(VK_SHADER_STAGE_FRAGMENT_BIT, SHADER_DIR "point_light.frag.spv")
			.addColorAttachment(VK_FORMAT_R16G16B16A16_SFLOAT, true)
			.setDepthAttachment(VK_FORMAT_D32_SFLOAT)
			.setDepthTest(true, false)
			.setVertexBindingDescriptions({})   // Clear default vertex binding
			.setVertexAttributeDescriptions({}) // Clear default vertex attributes
			.buildUnique();
	}


	void PointLightRenderSystem::render(const RenderContext& ctx)
	{
		constexpr float pointLightScale = 0.002f;

		m_pipeline->bind(ctx.cmd);
		m_pipeline->bindDescriptorSet(ctx.cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, ctx.globalSet);

		auto view = ctx.scene.registry().view<GlobalTransform, PointLight>();
		for (auto&& [entity, transform, light] : view.each())
		{
			PointLightPushConstants push{
				.position = glm::vec4{ transform.location, 1.0 },
				.color = glm::vec4{ light.color, 1.0 },
				.radius = light.intensity * transform.scale.x * pointLightScale
			};
			m_pipeline->pushConstants(ctx.cmd, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, push);

			vkCmdDraw(ctx.cmd, 6, 1, 0, 0);
		}
	}
}