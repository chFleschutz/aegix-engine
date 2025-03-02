#include "entity.h"

#include "scene/scene.h"
#include "scripting/script_base.h"

namespace Aegix::Scene
{
	Entity::Entity(entt::entity entityHandle, Scene* scene)
		: m_entityID(entityHandle), m_scene(scene)
	{
	}

	bool Entity::operator==(const Entity& other) const
	{
		return m_entityID == other.m_entityID && m_scene == other.m_scene;
	}

	bool Entity::operator!=(const Entity& other) const
	{
		return !(*this == other);
	}

	auto Entity::registry() const -> entt::registry&
	{
		return m_scene->m_registry;
	}

	void Entity::addScript(Scripting::ScriptBase* script)
	{
		script->m_entity = *this;
		m_scene->addScript(script);
	}
}