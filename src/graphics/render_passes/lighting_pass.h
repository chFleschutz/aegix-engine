#pragma once

#include "graphics/frame_graph/frame_graph.h"
#include "graphics/frame_graph/frame_graph_blackboard.h"

namespace Aegix::Graphics
{
	struct LightingData
	{
		FrameGraphResourceID sceneColor;
	};

	class LightingPass
	{
	public:
		LightingPass(FrameGraph& frameGraph, FrameGraphBlackboard& blackboard, FrameGraphResourceID sceneColor)
		{
			const auto& gBuffer = blackboard.get<GBufferData>();

			blackboard += frameGraph.addPass<LightingData>("Lighting",
				[&](FrameGraph::Builder& builder, LightingData& data)
				{
					data.sceneColor = builder.declareWrite(sceneColor);

					builder.declareRead(gBuffer.albedo);
					builder.declareRead(gBuffer.normal);
					builder.declareRead(gBuffer.depth);

				},
				[](const LightingData& data, const FrameGraphResourcePool& resources, const FrameInfo& frameInfo)
				{
					// Begin Rendering

					// Bind Pipeline

					// Bind Descriptor Sets

					// Draw Quad

					// End Rendering
				});
		}
	};
}