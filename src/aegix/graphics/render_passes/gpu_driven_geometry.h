#pragma once

#include "graphics/frame_graph/frame_graph_render_pass.h"

namespace Aegix::Graphics
{
	class GPUDrivenGeometry : public FGRenderPass
	{
	public:
		struct PushConstants
		{
			DescriptorHandle global;
			DescriptorHandle instance;
			DescriptorHandle visibility;
			uint32_t firstInstance;
			uint32_t instanceCount;
		};

		struct GlobalUBO
		{
			glm::mat4 projection;
			glm::mat4 view;
			glm::mat4 inverseView;
		};

		GPUDrivenGeometry(FGResourcePool& pool);

		virtual auto info() -> FGNode::Info override;
		virtual void execute(FGResourcePool& pool, const FrameInfo& frameInfo) override;

	private:
		void updateUBO(const FrameInfo& frameInfo);

		FGResourceHandle m_position;
		FGResourceHandle m_normal;
		FGResourceHandle m_albedo;
		FGResourceHandle m_arm;
		FGResourceHandle m_emissive;
		FGResourceHandle m_depth;
		FGResourceHandle m_visibleInstances;
		FGResourceHandle m_instanceData;
		FGResourceHandle m_drawBatches;

		BindlessFrameBuffer m_global;
	};
}