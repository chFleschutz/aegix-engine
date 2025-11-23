#pragma once

#include "graphics/frame_graph/frame_graph_resources.h"

namespace Aegix::Graphics
{
	class FGRenderPass;

	struct FGNode
	{
		struct Info
		{
			std::string name;
			std::vector<FGResourceHandle> reads;
			std::vector<FGResourceHandle> writes;
		};

		Info info;
		std::unique_ptr<FGRenderPass> pass;
		VkPipelineStageFlags srcStage{ 0 };
		VkPipelineStageFlags dstStage{ 0 };
		std::vector<VkBufferMemoryBarrier> bufferBarriers;
		std::vector<VkImageMemoryBarrier> imageBarriers;
		std::vector<FGTextureHandle> accessedTextures;
	};
}
