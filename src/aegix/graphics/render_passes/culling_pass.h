#pragma once

#include "graphics/draw_batch_registry.h"
#include "graphics/frame_graph/frame_graph_render_pass.h"
#include "graphics/pipeline.h"

namespace Aegix::Graphics
{
	class CullingPass : public FrameGraphRenderPass
	{
	public:
		static constexpr float INSTANCE_OVERALLOCATION = 1.5f;

		struct CullingPushConstants
		{
			// TODO: Add push constant members
		};

		CullingPass(DrawBatchRegistry& batcher);

		virtual auto createInfo(FrameGraphResourceBuilder& builder) -> FrameGraphNodeCreateInfo override;
		virtual void execute(FrameGraphResourcePool& resources, const FrameInfo& frameInfo) override;

	private:
		DrawBatchRegistry& m_drawBatcher;
		FrameGraphResourceHandle m_visibleDrawSet;
		Pipeline m_pipeline;
	};
}