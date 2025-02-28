#include "renderer_layer.h"

#include "core/engine.h"
#include "graphics/renderer.h"

#include "imgui.h"

namespace Aegix::UI
{
	void RendererLayer::onGuiRender()
	{
		if (!ImGui::Begin("Renderer"))
		{
			ImGui::End();
			return;
		}

		auto& renderer = Engine::instance().renderer();
		auto& frameGraph = renderer.frameGraph();
		auto& resourcePool = frameGraph.resourcePool();
		for (const auto& nodeHandle : frameGraph.nodes())
		{
			auto& node = resourcePool.node(nodeHandle);
			ImGui::Text(node.name.c_str());
		}
		
		ImGui::End();
	}
}
