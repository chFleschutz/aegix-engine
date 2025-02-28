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
		int i = 0;
		for (const auto& nodeHandle : frameGraph.nodes())
		{
			ImGui::PushID(i);

			auto& node = resourcePool.node(nodeHandle);
			if (ImGui::CollapsingHeader(node.name.c_str(), ImGuiTreeNodeFlags_None))
			{
				if (!node.inputs.empty() && ImGui::TreeNode("Inputs"))
				{
					for (const auto& inputHandle : node.inputs)
					{
						auto& input = resourcePool.resource(inputHandle);
						ImGui::BulletText("%s", input.name.c_str());
					}
					ImGui::TreePop();
				}

				if (!node.outputs.empty() && ImGui::TreeNode("Outputs"))
				{
					for (const auto& outputHandle : node.outputs)
					{
						auto& input = resourcePool.resource(outputHandle);
						ImGui::BulletText("%s", input.name.c_str());
					}
					ImGui::TreePop();
				}
			}

			ImGui::PopID();
			i++;
		}
		
		ImGui::End();
	}
}
