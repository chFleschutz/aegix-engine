#include "entity.h"

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
}