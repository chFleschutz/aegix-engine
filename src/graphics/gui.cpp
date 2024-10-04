#include "gui.h"

#include <cassert>

namespace Aegix::Graphics
{
	GUI::~GUI()
	{
		for (auto& layer : m_layers)
		{
			layer->onDetach();
		}
	}

	void GUI::update(float deltaTime)
	{
		// Cant use iterator because its possible to push/pop layers during update
		for (int i = 0; i < m_layers.size(); i++)
		{
			m_layers[i]->onUpdate(deltaTime);
		}
	}

	void GUI::renderGui()
	{
		// Cant use iterator because its possible to push/pop layers during update
		for (int i = 0; i < m_layers.size(); i++)
		{
			m_layers[i]->onGuiRender();
		}
	}

	void GUI::pushLayer(std::shared_ptr<Layer> layer)
	{
		assert(layer != nullptr && "Adding Layer failed: Layer is nullptr");
		m_layers.emplace_back(layer);
		layer->onAttach();
	}

	void GUI::popLayer(std::shared_ptr<Layer> layer)
	{
		auto it = std::find(m_layers.begin(), m_layers.end(), layer);
		if (it != m_layers.end())
		{
			(*it)->onDetach();
			m_layers.erase(it);
		}
	}
}