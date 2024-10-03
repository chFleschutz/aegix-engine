#include "lighting_pass.h"

#include "scene/components.h"
#include "scene/entity.h"

namespace Aegix::Graphics
{
	void LightingPass::render(const NewFrameInfo& frameInfo)
	{
		auto& camera = frameInfo.scene.camera().getComponent<Component::Camera>();
		camera.aspect = frameInfo.aspectRatio;

		updateGlobalUBO(frameInfo, camera);

		//for (auto&& [_, system] : frameInfo.scene.activeRenderSystems)
		//{
		//	system->render(frameInfo);
		//}
	}

	void LightingPass::updateGlobalUBO(const NewFrameInfo& frameInfo, const Component::Camera& camera)
	{
		GlobalUbo ubo{};
	
		// Camera Matrices
		ubo.projection = camera.projectionMatrix;
		ubo.view = camera.viewMatrix;
		ubo.inverseView = camera.inverseViewMatrix;

		// Point Lights
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