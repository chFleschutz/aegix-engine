#include "renderer_layer.h"

#include "core/engine.h"
#include "graphics/frame_graph/frame_graph_resource_pool.h"
#include "graphics/renderer.h"

#include "imgui.h"

namespace Aegix::UI
{
	static auto toString(Graphics::FrameGraphResourceType type) -> const char*
	{
		switch (type)
		{
		case Graphics::FrameGraphResourceType::Buffer:
			return "Buffer";
		case Graphics::FrameGraphResourceType::Texture:
			return "Texture";
		case Graphics::FrameGraphResourceType::Reference:
			return "Reference";
		default:
			return "Unknown";
		}
	}

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

		// Renderer info
		ImGui::SeparatorText("Renderer Info");

		ImGui::Text("Extent: %d x %d", renderer.swapChain().extent().width, renderer.swapChain().extent().height);
		ImGui::Text("Aspect Ratio: %.2f", renderer.aspectRatio());

		ImGui::NewLine();
		ImGui::SeparatorText("Render Passes");

		// Frame graph render passes
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

		ImGui::NewLine();
		ImGui::SeparatorText("Frame Graph Resources");

		// Frame graph resources
		const ImGuiTableFlags flags = ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter |
			ImGuiTableFlags_BordersV | ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable |
			ImGuiTableFlags_Hideable | ImGuiTableFlags_SizingFixedFit;
		const ImVec2 outer_size = ImVec2(0.0f, ImGui::GetTextLineHeightWithSpacing() * 10);
		if (ImGui::BeginTable("Resources", 4, flags, outer_size))
		{
			ImGui::TableSetupScrollFreeze(0, 1); // Make top row always visible
			ImGui::TableSetupColumn("ID", ImGuiTableColumnFlags_None);
			ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch);
			ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_None);
			ImGui::TableSetupColumn("Producer", ImGuiTableColumnFlags_None);
			ImGui::TableHeadersRow();

			for (const auto& res : resourcePool.resources())
			{
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Text("%d", res.handle.id);
				ImGui::TableSetColumnIndex(1);
				ImGui::Text("%s", res.name.c_str());
				ImGui::TableSetColumnIndex(2);
				ImGui::Text("%s", toString(res.type));
				ImGui::TableSetColumnIndex(3);
				ImGui::Text("%s", resourcePool.node(res.producer).name.c_str());
			}

			ImGui::EndTable();
		}

		ImGui::End();
	}
}
