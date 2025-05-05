#pragma once

#include "graphics/frame_graph/frame_graph.h"
#include "graphics/frame_graph/frame_graph_render_pass.h"

namespace Aegix::Graphics
{
	struct GBufferUbo
	{
		glm::mat4 projection{ 1.0f };
		glm::mat4 view{ 1.0f };
		glm::mat4 inverseView{ 1.0f };
	};

	class GeometryPass : public FrameGraphRenderPass
	{
	public:
		GeometryPass(FrameGraph& framegraph);

		virtual auto createInfo(FrameGraphResourceBuilder& builder) -> FrameGraphNodeCreateInfo override;
		virtual void execute(FrameGraphResourcePool& resources, const FrameInfo& frameInfo, const RenderContext& ctx) override;

	private:
		void updateUBO(RenderStage& stage, const FrameInfo& frameInfo, const RenderContext& ctx);

		FrameGraphResourceHandle m_position;
		FrameGraphResourceHandle m_normal;
		FrameGraphResourceHandle m_albedo;
		FrameGraphResourceHandle m_arm;
		FrameGraphResourceHandle m_emissive;
		FrameGraphResourceHandle m_depth;
	};
}