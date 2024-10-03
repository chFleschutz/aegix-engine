#pragma once

#include "graphics/descriptors.h"
#include "graphics/renderpasses/render_pass.h"
#include "graphics/uniform_buffer.h"

namespace Aegix::Graphics
{
	class LightingPass : public RenderPass
	{
	public:
		LightingPass() = default;
		~LightingPass() = default;

		void render(const FrameInfo& frameInfo) override;
	};
}