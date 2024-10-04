#include "lighting_pass.h"

#include "graphics/systems/render_system_collection.h"
#include "scene/components.h"
#include "scene/entity.h"

namespace Aegix::Graphics
{
	LightingPass::LightingPass(VulkanDevice& device, DescriptorPool& pool)
	{
		m_globalSetLayout = DescriptorSetLayout::Builder(device)
			.addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
			.build();

		m_globalUBO = std::make_unique<UniformBuffer<GlobalUbo>>(device);

		m_globalDescriptorSet = DescriptorSet::Builder(device, pool, *m_globalSetLayout)
			.addBuffer(0, *m_globalUBO)
			.build();
	}

	void LightingPass::render(FrameInfo& frameInfo)
	{
		// TODO: Update aspect ration only on resize
		auto& camera = frameInfo.scene.camera().getComponent<Component::Camera>();
		camera.aspect = frameInfo.aspectRatio;

		// TODO: Bind global descriptor set here instead of in each render system
		frameInfo.globalDescriptorSet = m_globalDescriptorSet->descriptorSet(frameInfo.frameIndex);

		updateGlobalUBO(frameInfo);

		for (auto&& [_, system] : frameInfo.renderSystemCollection)
		{
			system->render(frameInfo);
		}
	}

	void LightingPass::updateGlobalUBO(const FrameInfo& frameInfo)
	{
		auto& camera = frameInfo.scene.camera().getComponent<Component::Camera>();

		GlobalUbo ubo{};
		ubo.projection = camera.projectionMatrix;
		ubo.view = camera.viewMatrix;
		ubo.inverseView = camera.inverseViewMatrix;

		int lighIndex = 0;
		auto view = frameInfo.scene.viewEntities<Aegix::Component::Transform, Aegix::Component::PointLight>();
		for (auto&& [entity, transform, pointLight] : view.each())
		{
			assert(lighIndex < GlobalLimits::MAX_LIGHTS && "Point lights exceed maximum number of point lights");
			ubo.pointLights[lighIndex].position = glm::vec4(transform.location, 1.0f);
			ubo.pointLights[lighIndex].color = glm::vec4(pointLight.color, pointLight.intensity);
			lighIndex++;
		}
		ubo.numLights = lighIndex;

		m_globalUBO->setData(frameInfo.frameIndex, ubo);
	}
}