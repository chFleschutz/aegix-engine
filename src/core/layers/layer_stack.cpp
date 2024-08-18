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
		// Cant use iterator because its possible to push/pop layers during update
		for (int i = 0; i < m_layers.size(); i++) 
		{
			m_layers[i]->onUpdate(deltaTime);
		}
	}

	void LayerStack::renderGui()
	{
		// Cant use iterator because its possible to push/pop layers during update
		for (int i = 0; i < m_layers.size(); i++)
		{
			m_layers[i]->onGuiRender();
		}
	}
}
