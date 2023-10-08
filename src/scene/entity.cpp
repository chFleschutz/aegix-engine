#include "entity.h"


namespace vre
{
	Entity::Entity(entt::entity entityHandle, Scene* scene)
		: m_entityHandle{ entityHandle }, m_scene{ scene }
	{
	}

} // namespace vre
