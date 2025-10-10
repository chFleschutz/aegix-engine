#pragma once

#include "graphics/render_systems/render_system.h"

namespace Aegix::Graphics
{
	//using RenderSystemList = std::vector<std::unique_ptr<RenderSystem>>;

	//class RenderSystemRegistry
	//{
	//public:
	//	RenderSystemRegistry() = default;
	//	RenderSystemRegistry(const RenderSystemRegistry&) = delete;
	//	RenderSystemRegistry(RenderSystemRegistry&&) = delete;
	//	~RenderSystemRegistry() = default;

	//	auto operator=(const RenderSystemRegistry&) -> RenderSystemRegistry& = delete;
	//	auto operator=(RenderSystemRegistry&&) -> RenderSystemRegistry& = delete;

	//	template <typename T>
	//		requires std::is_base_of_v<RenderSystem, T> && std::is_default_constructible_v<T>
	//	void add(RenderStageType stage)
	//	{
	//		AGX_ASSERT_X(stage < RenderStageType::Count, "Invalid render stage");
	//		m_renderSystems[static_cast<size_t>(stage)].emplace_back(std::make_unique<T>());
	//	}

	//	auto get(RenderStageType stage) -> RenderSystemList&
	//	{
	//		AGX_ASSERT_X(stage < RenderStageType::Count, "Invalid render stage");
	//		return m_renderSystems[static_cast<size_t>(stage)];
	//	}

	//private:
	//	std::array<RenderSystemList, static_cast<size_t>(RenderStageType::Count)> m_renderSystems;
	//};
}