#pragma once

#include "core/systems/system.h"
#include "scene/scene.h"

#include <cassert>
#include <memory>
#include <type_traits>
#include <typeindex>
#include <unordered_map>

namespace Aegix
{
	class SystemManager
	{
	public:
		SystemManager() = default;
		~SystemManager();

		template<typename T, typename... Args>
		T& add(Args&&... args)
		{
			static_assert(std::is_base_of_v<System, T>, "T must be a subclass of System");
			std::type_index index(typeid(T));
			assert(!m_systems.contains(index) && "Manager already contains T");
			auto system = std::make_unique<T>(std::forward<Args>(args)...);
			system->onAttach();
			m_systems[index] = std::move(system);
			return *static_cast<T*>(m_systems[index].get());
		}

		template<typename T>
		bool has()
		{
			static_assert(std::is_base_of_v<System, T>, "T must be a subclass of System");
			std::type_index index(typeid(T));
			return m_systems.contains(index);
		}

		template<typename T>
		T& get()
		{
			static_assert(std::is_base_of_v<System, T>, "T must be a subclass of System");
			std::type_index index(typeid(T));
			auto it = m_systems.find(index);
			assert(it != m_systems.end() && "Manager does not contain T");
			return *static_cast<T*>(it->second.get());
		}

		template<typename T>
		void remove()
		{
			static_assert(std::is_base_of_v<System, T>, "T must be a subclass of System");
			std::type_index index(typeid(T));
			auto it = m_systems.find(index);
			assert(it != m_systems.end() && "Manager does not contain T");
			it->second->onDetach();
			m_systems.erase(it);
		}

		void update(float deltaSeconds, Scene::Scene& scene);
		
	private:
		std::unordered_map<std::type_index, std::unique_ptr<System>> m_systems;
	};
}