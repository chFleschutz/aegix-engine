#pragma once

#include "graphics/render_systems/render_system.h"

namespace Aegix::Graphics
{
	class RenderSystemRegistry
	{
	public:
		RenderSystemRegistry() = default;
		RenderSystemRegistry(const RenderSystemRegistry&) = delete;
		RenderSystemRegistry(RenderSystemRegistry&&) = delete;
		~RenderSystemRegistry() = default;

		auto operator=(const RenderSystemRegistry&) -> RenderSystemRegistry& = delete;
		auto operator=(RenderSystemRegistry&&) -> RenderSystemRegistry& = delete;

		template <typename T>
			requires std::is_base_of_v<RenderSystemBase, T>
		void add(RenderStageType stage)
		{
			m_renderSystems[stage].emplace_back(std::make_unique<T>());
		}

		auto get(RenderStageType stage) -> std::vector<std::unique_ptr<RenderSystemBase>>&
		{
			return m_renderSystems[stage];
		}

	private:
		std::unordered_map<RenderStageType, std::vector<std::unique_ptr<RenderSystemBase>>> m_renderSystems;
	};
}