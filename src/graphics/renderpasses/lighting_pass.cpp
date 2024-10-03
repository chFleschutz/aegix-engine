#include "lighting_pass.h"

#include "scene/components.h"
#include "scene/entity.h"

namespace Aegix::Graphics
{
	void LightingPass::render(const FrameInfo& frameInfo)
	{
		auto& camera = frameInfo.scene.camera().getComponent<Component::Camera>();
		camera.aspect = frameInfo.aspectRatio;

		//for (auto&& [_, system] : frameInfo.scene.activeRenderSystems)
		//{
		//	system->render(frameInfo);
		//}
	}
}