#pragma once

#include "graphics/frame_graph/frame_graph.h"
#include "graphics/frame_graph/frame_graph_blackboard.h"

#include <iostream>

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
					std::cout << "Creating GBuffer pass\n";

					data.albedo = builder.create<FrameGraphTexture>("Albedo", { 1920, 1080 });
					data.normal = builder.create<FrameGraphTexture>("Normal", { 1920, 1080 });
					data.depth = builder.create<FrameGraphTexture>("Depth", { 1920, 1080 });
					builder.declareWrite(data.albedo);
					builder.declareWrite(data.normal);
					builder.declareWrite(data.depth);
				},
				[=](const GBufferData& data)
				{
					std::cout << "Executing GBuffer pass\n";
				});
		}
	};



	struct LightingData
	{
		FrameGraphResourceID lighting;
	};

	class LightingPass
	{
	public:
		LightingPass(FrameGraph& frameGraph, FrameGraphBlackboard& blackboard)
		{
			const auto& gBuffer = blackboard.get<GBufferData>();

			blackboard += frameGraph.addPass<LightingData>("Lighting",
				[&](FrameGraph::Builder& builder, LightingData& data)
				{
					std::cout << "Creating Lighting pass\n";

					data.lighting = builder.create<FrameGraphTexture>("Lighting", { 1920, 1080 });
					builder.declareWrite(data.lighting);

					builder.declareRead(gBuffer.albedo);
					builder.declareRead(gBuffer.normal);
					builder.declareRead(gBuffer.depth);
				},
				[=](const LightingData& data)
				{
					std::cout << "Executing Lighting pass\n";
				});
		}
	};
}