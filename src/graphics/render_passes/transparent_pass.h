#pragma once

#include "graphics/frame_graph/frame_graph_render_pass.h"

namespace Aegix::Graphics
{
	class TransparentPass : public FrameGraphRenderPass
	{
	public:
		TransparentPass()
		{
		}

		virtual auto createInfo(FrameGraphResourcePool& pool) -> FrameGraphNodeCreateInfo override
		{
			m_sceneColor = pool.addResource({ "SceneColor", FrameGraphResourceType::Reference });
			m_depth = pool.addResource({ "Depth", FrameGraphResourceType::Reference });

			return FrameGraphNodeCreateInfo{
				.name = "Transparent Pass",
				.inputs = { m_sceneColor },
				.outputs = { m_sceneColor }
			};
		}

		virtual void execute(FrameGraphResourcePool& resources, const FrameInfo& frameInfo) override
		{
			// Begin rendering

			// Draw transparent objects

			// End rendering
		}

	private:
		FrameGraphResourceHandle m_sceneColor;
		FrameGraphResourceHandle m_depth;
	};
}