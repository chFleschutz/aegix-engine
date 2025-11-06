#pragma once

#include "graphics/bindless/bindless_buffer.h"
#include "graphics/frame_graph/frame_graph.h"
#include "graphics/frame_graph/frame_graph_render_pass.h"
#include "graphics/render_systems/render_system.h"

namespace Aegix::Graphics
{
	struct GBufferUbo
	{
		glm::mat4 projection{ 1.0f };
		glm::mat4 view{ 1.0f };
		glm::mat4 inverseView{ 1.0f };
	};

	class GeometryPass : public FrameGraphRenderPass
	{
	public:
		GeometryPass(FrameGraph& framegraph);

		virtual auto createInfo(FrameGraphResourceBuilder& builder) -> FrameGraphNodeCreateInfo override;
		virtual void execute(FrameGraphResourcePool& resources, const FrameInfo& frameInfo) override;

		template<typename T, typename... Args>
			requires std::is_base_of_v<RenderSystem, T>&& std::is_constructible_v<T, Args...>
		auto addRenderSystem(Args&&... args) -> T&
		{
			m_renderSystems.emplace_back(std::make_unique<T>(std::forward<Args>(args)...));
			return static_cast<T&>(*m_renderSystems.back());
		}

	private:
		auto createDescriptorSetLayout() -> DescriptorSetLayout;

		void updateUBO(const FrameInfo& frameInfo);

		FrameGraphResourceHandle m_position;
		FrameGraphResourceHandle m_normal;
		FrameGraphResourceHandle m_albedo;
		FrameGraphResourceHandle m_arm;
		FrameGraphResourceHandle m_emissive;
		FrameGraphResourceHandle m_depth;

		std::vector<std::unique_ptr<RenderSystem>> m_renderSystems;

		BindlessFrameBuffer m_globalUbo;
		DescriptorSetLayout m_globalSetLayout;
		std::vector<DescriptorSet> m_globalSets;
	};
}