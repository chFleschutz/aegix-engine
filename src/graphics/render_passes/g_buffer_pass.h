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
		GBufferPass(FrameGraph& frameGraph, FrameGraphBlackboard& blackboard)
		{
			blackboard += frameGraph.addPass<GBufferData>("GBuffer",
				[&](FrameGraph::Builder& builder, GBufferData& data)
				{
					//data.albedo = builder.create<FrameGraphTexture>("Albedo", { 1920, 1080 });
					//data.normal = builder.create<FrameGraphTexture>("Normal", { 1920, 1080 });
					//data.depth = builder.create<FrameGraphTexture>("Depth", { 1920, 1080 });
					//builder.declareWrite(data.albedo);
					//builder.declareWrite(data.normal);
					//builder.declareWrite(data.depth);
				},
				[=](const GBufferData& data, const FrameInfo& frameInfo)
				{
				});
		}
	};
}