#pragma once

#include "graphics/descriptors.h"
#include "graphics/frame_graph/render_stage.h"
#include "graphics/render_context.h"
#include "graphics/pipeline.h"

namespace Aegix::Graphics
{
	/// @brief Base class for all render systems
	//class RenderSystem
	//{
	//public:
	//	RenderSystem(VkDescriptorSetLayout globalSetLayout);
	//	RenderSystem(const RenderSystem&) = delete;
	//	virtual ~RenderSystem() = default;

	//	RenderSystem& operator=(const RenderSystem&) = delete;

	//	DescriptorSetLayout& descriptorSetLayout() { return *m_descriptorSetLayout; }

	//	virtual void render(const RenderContext& ctx, VkDescriptorSet globalSet) = 0;

	//protected:
	//	std::unique_ptr<DescriptorSetLayout> m_descriptorSetLayout;
	//	std::unique_ptr<Pipeline> m_pipeline;
	//};

	//template<typename T>
	//concept RenderSystemDerived = std::derived_from<T, RenderSystem>;

	//template <typename T>
	//concept ValidRenderSystem = 
	//	RenderSystemDerived<T> &&
	//	requires { { T::STAGE } -> std::convertible_to<RenderStage::Type>; };

	//template<typename T>
	//concept ValidMaterial = 
	//	requires { typename T::Instance; } && 
	//	requires { typename T::RenderSystem; } &&
	//	RenderSystemDerived<typename T::RenderSystem>;
}
