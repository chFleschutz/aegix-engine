#pragma once

#include "graphics/render_context.h"

namespace Aegix::Graphics
{
	enum class RenderStageType
	{
		Opaque,
		Transparent,

		Count
	};

	class RenderSystemBase
	{
	public:
		virtual ~RenderSystemBase() = default;

		virtual void render(const RenderContext& ctx) = 0;
	};
}