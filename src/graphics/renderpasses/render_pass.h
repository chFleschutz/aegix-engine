#pragma once

#include "graphics/frame_info.h"
#include "scene/scene.h"

namespace Aegix::Graphics
{
	class RenderPass
	{
	public:
		virtual ~RenderPass() = default;

		virtual void render(const NewFrameInfo& frameInfo) = 0;
	};
}