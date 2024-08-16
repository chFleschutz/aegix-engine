#pragma once

#include "core/layers/layer.h"

#include <memory>
#include <vector>
#include <cassert>

namespace Aegix
{
	class LayerStack
	{
	public:
		LayerStack() = default;
		~LayerStack()
		{
			for (auto& layer : m_layers)
			{
				layer->onDetach();
			}
		}

		template<typename T, typename... Args>
		std::shared_ptr<T> push(Args&&... args)
		{
			auto layer = std::make_shared<T>(std::forward<Args>(args)...);
			push(layer);
			return layer;
		}

		void push(std::shared_ptr<Layer> layer)
		{
			assert(layer != nullptr && "Adding Layer failed: Layer is nullptr");
			m_layers.emplace_back(layer);
			layer->onAttach();
		}

		void pop(std::shared_ptr<Layer> layer)
		{
			auto it = std::find(m_layers.begin(), m_layers.end(), layer);
			if (it != m_layers.end())
			{
				m_layers.erase(it);
			}
		}

		void update(float deltaTime)
		{
			for (auto& layer : m_layers)
			{
				layer->onUpdate(deltaTime);
			}
		}

		void renderGui()
		{
			for (auto& layer : m_layers)
			{
				layer->onGuiRender();
			}
		}

	private:
		std::vector<std::shared_ptr<Layer>> m_layers;
	};
}