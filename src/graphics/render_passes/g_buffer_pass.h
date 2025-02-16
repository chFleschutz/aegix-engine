#pragma once

#include "graphics/frame_graph/frame_graph.h"
#include "graphics/frame_graph/frame_graph_blackboard.h"

namespace Aegix::Graphics
{
	struct GBufferData
	{
		FrameGraphResourceID albedo;
		FrameGraphResourceID normal;
		FrameGraphResourceID depth;
	};

	class GBufferPass
	{
	public:
		GBufferPass(FrameGraph& frameGraph, FrameGraphBlackboard& blackboard, 
			FrameGraphResourceID albedo, FrameGraphResourceID normal, FrameGraphResourceID depth )
		{
			blackboard += frameGraph.addPass<GBufferData>("GBuffer",
				[&](FrameGraph::Builder& builder, GBufferData& data)
				{
					data.albedo = builder.declareWrite(albedo);
					data.normal = builder.declareWrite(normal);
					data.depth = builder.declareWrite(depth);
				},
				[=](const GBufferData& data, const FrameGraphResourcePool& resources, const FrameInfo& frameInfo)
				{
					const auto& albedo = resources.getTexture(data.albedo);
					const auto& normal = resources.getTexture(data.normal);
					const auto& depth = resources.getTexture(data.depth);

					// TODO: Implement GBuffer pass
				});
		}
	};
}