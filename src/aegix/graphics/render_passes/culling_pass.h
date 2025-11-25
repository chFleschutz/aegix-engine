#pragma once

#include "graphics/bindless/descriptor_handle.h"
#include "graphics/draw_batch_registry.h"
#include "graphics/frame_graph/frame_graph_render_pass.h"
#include "graphics/pipeline.h"

namespace Aegix::Graphics
{
	class CullingPass : public FGRenderPass
	{
	public:
		static constexpr float INSTANCE_OVERALLOCATION = 1.5f;
		static constexpr uint32_t WORKGROUP_SIZE = 64;

		struct CullingPushConstants
		{
			DescriptorHandle instanceBuffer;
			DescriptorHandle drawBatchBuffer;
			DescriptorHandle visibilityBuffer;
			DescriptorHandle visibleCountBuffer;
			uint32_t instanceCount;
		};

		CullingPass(FGResourcePool& pool, DrawBatchRegistry& batcher);

		virtual auto info() -> FGNode::Info override;
		virtual void execute(FGResourcePool& pool, const FrameInfo& frameInfo) override;

	private:
		DrawBatchRegistry& m_drawBatcher;
		FGResourceHandle m_instanceBuffer;
		FGResourceHandle m_drawBatchBuffer;
		FGResourceHandle m_visibleIndices;
		FGResourceHandle m_visibleCounts;
		Pipeline m_pipeline;
	};
}