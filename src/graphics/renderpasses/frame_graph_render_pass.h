#pragma once

#include "graphics/frame_info.h"

namespace Aegix::Graphics
{
	class FrameGraphRenderPass
	{
	public:
		virtual ~FrameGraphRenderPass() = default;

		virtual void execute(const FrameInfo& frameInfo) = 0;
	};
}