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
		static constexpr uint32_t WORKGROUP_SIZE = 64;

		struct CullingPushConstants
		{
			DescriptorHandle cameraData;
			DescriptorHandle staticInstances;
			DescriptorHandle dynamicInstances;
			DescriptorHandle drawBatches;
			DescriptorHandle visibilityInstances;
			DescriptorHandle indirectDrawCommands;
			DescriptorHandle indirectDrawCounts;
			uint32_t staticInstanceCount;
			uint32_t dynamicInstanceCount;
		};

		CullingPass(FGResourcePool& pool, DrawBatchRegistry& batcher);

		virtual auto info() -> FGNode::Info override;
		virtual void execute(FGResourcePool& pool, const FrameInfo& frameInfo) override;

	private:
		DrawBatchRegistry& m_drawBatcher;
		FGResourceHandle m_cameraData;
		FGResourceHandle m_staticInstances;
		FGResourceHandle m_dynamicInstances;
		FGResourceHandle m_drawBatchBuffer;
		FGResourceHandle m_visibleIndices;
		FGResourceHandle m_indirectDrawCommands;
		FGResourceHandle m_indirectDrawCounts;
		Pipeline m_pipeline;
	};
}