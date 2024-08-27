#include "system_manager.h"

#include "core/systems/system.h"

namespace Aegix
{
	SystemManager::~SystemManager()
	{
		for (auto& [type, system] : m_systems)
		{
			system->onDetach();
		}
	}

	void SystemManager::update(float deltaSeconds, Scene::Scene& scene)
	{
		for (auto& [type, system] : m_systems)
		{
			system->onUpdate(deltaSeconds, scene);
		}
	}
}