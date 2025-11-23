#pragma once

#include "graphics/draw_batch_registry.h"
#include "graphics/frame_graph/frame_graph_render_pass.h"
#include "graphics/pipeline.h"

namespace Aegix::Graphics
{
	class CullingPass : public FGRenderPass
	{
	public:
		static constexpr float INSTANCE_OVERALLOCATION = 1.5f;

		struct CullingPushConstants
		{
			// TODO: Add push constant members
		};

		CullingPass(FGResourcePool& pool, DrawBatchRegistry& batcher);

		virtual auto info() -> FGNode::Info override;
		virtual void execute(FGResourcePool& pool, const FrameInfo& frameInfo) override;

	private:
		DrawBatchRegistry& m_drawBatcher;
		FGResourceHandle m_visibleDrawSet;
		Pipeline m_pipeline;
	};
}