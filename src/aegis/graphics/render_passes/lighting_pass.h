#pragma once

#include "graphics/frame_graph/frame_graph_render_pass.h"
#include "graphics/pipeline.h"
#include "graphics/descriptors.h"

namespace Aegis::Graphics
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

	class LightingPass : public FGRenderPass
	{
	public:
		LightingPass(FGResourcePool& pool);

		virtual auto info() -> FGNode::Info override;
		virtual void execute(FGResourcePool& pool, const FrameInfo& frameInfo) override;
		virtual void drawUI() override;

	private:
		auto createGBufferSetLayout() -> DescriptorSetLayout;
		auto createIBLSetLayout() -> DescriptorSetLayout;
		void updateLightingUBO(const FrameInfo& frameInfo);

		FGResourceHandle m_sceneColor;
		FGResourceHandle m_position;
		FGResourceHandle m_normal;
		FGResourceHandle m_albedo;
		FGResourceHandle m_arm;
		FGResourceHandle m_emissive;
		FGResourceHandle m_ssao;

		LightingViewMode m_viewMode{ LightingViewMode::SceneColor };
		float m_ambientOcclusionFactor{ 1.0f };

		std::unique_ptr<Pipeline> m_pipeline;

		DescriptorSetLayout m_gbufferSetLayout;
		DescriptorSetLayout m_iblSetLayout;
		std::vector<DescriptorSet> m_gbufferSets;
		std::vector<DescriptorSet> m_iblSets;
		Buffer m_ubo;
	};
}