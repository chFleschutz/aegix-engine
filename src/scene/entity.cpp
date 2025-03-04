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

	void Entity::setParent(Entity parent)
	{
		parent.addChild(*this);
	}

	void Entity::addChild(Entity child)
	{
		auto& parent = child.getOrAddComponent<Parent>();
		if (parent.entity == *this) // Already added as a child
			return;

		parent.entity = *this;

		auto& children = getOrAddComponent<Children>();
		if (children.first)
		{
			children.last.getOrAddComponent<Siblings>().next = child;
			child.getOrAddComponent<Siblings>().prev = children.last;
		}
		else // No children yet
		{
			children.first = child;
		}
		children.last = child;
		children.count++;
	}

	void Entity::removeChild(Entity child)
	{
		assert(hasComponent<Children>() && "Cannot remove child: Entity does not have children");
		assert(child.hasComponent<Parent>() && "Cannot remove child: Entity does not have a parent");
		assert(child.component<Parent>().entity == *this && "Cannot remove child: Entity is not a child of this entity");

		child.component<Parent>().entity = Entity{};

		auto& children = component<Children>();
		children.count--;

		// Remove child from siblings linked list
		auto& siblings = child.getOrAddComponent<Siblings>();
		if (siblings.prev)
		{
			siblings.prev.getOrAddComponent<Siblings>().next = siblings.next;
		}
		else // first child
		{
			children.first = siblings.prev;
		}

		if (siblings.next)
		{
			siblings.next.getOrAddComponent<Siblings>().prev = siblings.prev;
		}
		else // last child
		{
			children.last = siblings.next;
		}
		
		siblings.prev = Entity{};
		siblings.next = Entity{};
	}
}