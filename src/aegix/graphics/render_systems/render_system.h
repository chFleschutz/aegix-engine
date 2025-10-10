#pragma once

#include "graphics/render_context.h"

namespace Aegix::Graphics
{
	class RenderSystem
	{
	public:
		RenderSystem() = default;
		virtual ~RenderSystem() = default;

		virtual void render(const RenderContext& ctx) = 0;
	};
}