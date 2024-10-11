#include "frame_graph.h"

namespace Aegix::Graphics
{
	FrameGraph::FrameGraph()
	{
	}

	void FrameGraph::render(FrameInfo& frameInfo)
	{
		for (auto& node : m_nodes)
		{
			assert(node.renderPass && "RenderPass is not initialized");

			node.renderPass->execute(frameInfo);
		}
	}
}