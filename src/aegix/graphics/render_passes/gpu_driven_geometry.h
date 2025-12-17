#pragma once

#include "graphics/frame_graph/frame_graph_render_pass.h"

namespace Aegix::Graphics
{
	class GPUDrivenGeometry : public FGRenderPass
	{
	public:
		struct PushConstant
		{
			DescriptorHandle cameraData;
			DescriptorHandle staticInstances;
			DescriptorHandle dynamicInstances;
			DescriptorHandle visibility;
			uint32_t batchFirstID;
			uint32_t batchSize;
			uint32_t staticCount;
			uint32_t dynamicCount;
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
		FGResourceHandle m_staticInstanceData;
		FGResourceHandle m_dynamicInstanceData;
		FGResourceHandle m_drawBatches;
		FGResourceHandle m_indirectDrawCommands;
		FGResourceHandle m_indirectDrawCounts;
		FGResourceHandle m_cameraData;
	};
}