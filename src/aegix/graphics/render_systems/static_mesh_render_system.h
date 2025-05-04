#pragma once

#include "graphics/render_systems/render_system.h"

namespace Aegix::Graphics
{
	class StaticMeshRenderSystem : public RenderSystemBase
	{
	public:
		virtual void render(const FrameInfo& frameInfo, VkDescriptorSet globalSet) override
		{
			// TODO
		}

	private:

	};
}