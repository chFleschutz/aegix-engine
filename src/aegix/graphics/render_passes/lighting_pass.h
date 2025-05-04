#pragma once

#include "graphics/frame_graph/frame_graph_render_pass.h"
#include "graphics/pipeline.h"

namespace Aegix::Graphics
{
	enum class LightingViewMode : int32_t
	{
		SceneColor = 0,
		Albedo = 1,
		AmbientOcclusion = 2,
		Roughness = 3,
		Metallic = 4,
		Emissive = 5,
	};

	struct LightingUniforms
	{
		struct AmbientLight
		{
			glm::vec4 color{ 0.0f };
		};

		struct DirectionalLight
		{
			glm::vec4 direction{ 0.0f, 0.0f, 1.0f, 0.0f };
			glm::vec4 color{ 0.0f };
		};

		struct PointLight
		{
			glm::vec4 position{ 0.0f };
			glm::vec4 color{ 0.0f };
		};

		glm::vec4 cameraPosition{ 0.0f };
		AmbientLight ambient{};
		DirectionalLight directional{};
		std::array<PointLight, MAX_POINT_LIGHTS> pointLights{};
		int32_t pointLightCount{ 0 };
		float ambientOcclusionFactor{ 0.5f };
		LightingViewMode viewMode{ LightingViewMode::SceneColor };
	};

	class LightingPass : public FrameGraphRenderPass
	{
	public:
		LightingPass();

		virtual auto createInfo(FrameGraphResourceBuilder& builder) -> FrameGraphNodeCreateInfo override;
		virtual void execute(FrameGraphResourcePool& resources, const FrameInfo& frameInfo, const RenderContext& ctx) override;
		virtual void drawUI() override;

	private:
		void updateLightingUBO(const RenderContext& frameInfo);

		FrameGraphResourceHandle m_sceneColor;
		FrameGraphResourceHandle m_position;
		FrameGraphResourceHandle m_normal;
		FrameGraphResourceHandle m_albedo;
		FrameGraphResourceHandle m_arm;
		FrameGraphResourceHandle m_emissive;
		FrameGraphResourceHandle m_ssao;

		LightingViewMode m_viewMode{ LightingViewMode::SceneColor };
		float m_ambientOcclusionFactor{ 1.0f };

		std::unique_ptr<Pipeline> m_pipeline;

		std::unique_ptr<DescriptorSetLayout> m_gbufferSetLayout;
		std::unique_ptr<DescriptorSet> m_gbufferSet;
		Buffer m_ubo;

		// IBL
		std::unique_ptr<DescriptorSetLayout> m_iblSetLayout;
		std::unique_ptr<DescriptorSet> m_iblSet;
	};
}