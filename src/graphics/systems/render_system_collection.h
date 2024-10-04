#pragma once

#include "graphics/systems/render_system.h"

#include <unordered_map>
#include <typeindex>

namespace Aegix::Graphics
{
	class RenderSystemCollection
	{
	public:
		RenderSystemCollection() = default;
		~RenderSystemCollection() = default;

		template<typename T>
		RenderSystem& addRenderSystem(VulkanDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout setLayout)
		{
			static_assert(std::is_base_of_v<RenderSystem, T>, "T has to be a subclass of RenderSystem");

			auto it = m_renderSystems.find(typeid(T));
			if (it != m_renderSystems.end())
				return *it->second;

			auto newSystem = std::make_unique<T>(device, renderPass, setLayout);
			return *m_renderSystems.emplace(typeid(T), std::move(newSystem)).first->second;
		}

		auto begin() const { return m_renderSystems.begin(); }
		auto end() const { return m_renderSystems.end(); }

	private:
		std::unordered_map<std::type_index, std::unique_ptr<RenderSystem>> m_renderSystems;
	};
}