#pragma once

#include "graphics/frame_graph/frame_graph_render_pass.h"

namespace Aegix::Graphics
{
	class GPUDrivenGeometry : public FGRenderPass
	{
	public:
		struct PushConstants
		{
			uint32_t test;
		};

		GPUDrivenGeometry(FGResourcePool& pool);

		virtual auto info() -> FGNode::Info override;
		virtual void execute(FGResourcePool& pool, const FrameInfo& frameInfo) override;

	private:
		FGResourceHandle m_position;
		FGResourceHandle m_normal;
		FGResourceHandle m_albedo;
		FGResourceHandle m_arm;
		FGResourceHandle m_emissive;
		FGResourceHandle m_depth;
		FGResourceHandle m_visibleInstances;
		FGResourceHandle m_instanceData;
	};
}