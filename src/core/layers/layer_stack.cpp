#include "layer_stack.h"

#include "core/layers/layer.h"

#include <cassert>

namespace Aegix
{
	LayerStack::~LayerStack()
	{
		for (auto& layer : m_layers)
		{
			layer->onDetach();
		}
	}

	void LayerStack::push(std::shared_ptr<Layer> layer)
	{
		assert(layer != nullptr && "Adding Layer failed: Layer is nullptr");
		m_layers.emplace_back(layer);
		layer->onAttach();
	}

	void LayerStack::pop(std::shared_ptr<Layer> layer)
	{
		auto it = std::find(m_layers.begin(), m_layers.end(), layer);
		if (it != m_layers.end())
		{
			m_layers.erase(it);
		}
	}

	void LayerStack::update(float deltaTime)
	{
		for (auto& layer : m_layers)
		{
			layer->onUpdate(deltaTime);
		}
	}

	void LayerStack::renderGui()
	{
		for (auto& layer : m_layers)
		{
			layer->onGuiRender();
		}
	}
}
