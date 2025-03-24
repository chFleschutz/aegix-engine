#include "pch.h"

#include "entity.h"

#include "scene/scene.h"
#include "scripting/script_base.h"

namespace Aegix::Scene
{
	Entity::Entity(entt::entity entityHandle, Scene* scene)
		: m_id(entityHandle), m_scene(scene)
	{
	}

	bool Entity::operator==(const Entity& other) const
	{
		return m_id == other.m_id && m_scene == other.m_scene;
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
		AGX_ASSERT_X(*this, "Cannot set parent: Entity is null");
		AGX_ASSERT_X(parent, "Cannot set parent: Parent entity is null");
		AGX_ASSERT_X(parent != *this, "Cannot set parent: Entity cannot be its own parent");

		parent.addChild(*this);
	}

	void Entity::addChild(Entity child)
	{
		AGX_ASSERT_X(*this, "Cannot add child: Entity is null");
		AGX_ASSERT_X(child, "Cannot add child: Child entity is null");

		auto& parent = child.component<Parent>();
		if (parent.entity == *this) // Already added as a child
			return;

		parent.entity = *this;

		auto& children = component<Children>();
		if (children.first)
		{
			children.last.component<Siblings>().next = child;
			child.component<Siblings>().prev = children.last;
		}
		else // No children yet
		{
			children.first = child;
		}
		children.last = child;
		children.count++;
	}

	void Entity::removeParent()
	{
		AGX_ASSERT_X(*this, "Cannot remove parent: Entity is null");
		AGX_ASSERT_X(hasComponent<Parent>(), "Cannot remove parent: Entity does not have a parent");

		auto& parent = component<Parent>();
		if (!parent.entity)
			return;

		parent.entity.removeChild(Entity{ *this });
		parent.entity = Entity{};
	}

	void Entity::removeChild(Entity child)
	{
		AGX_ASSERT_X(*this, "Cannot remove child: Entity is null");
		AGX_ASSERT_X(hasComponent<Children>(), "Cannot remove child: Entity does not have children");
		AGX_ASSERT_X(child, "Cannot remove child: Child entity is null");
		AGX_ASSERT_X(child.hasComponent<Parent>(), "Cannot remove child: Entity does not have a parent");
		AGX_ASSERT_X(child.component<Parent>().entity == *this, "Cannot remove child: Entity is not a child of this entity");

		auto& children = component<Children>();
		children.count--;
		
		auto& siblings = child.component<Siblings>();
		Entity prevSibling = siblings.prev; 
		Entity nextSibling = siblings.next;
		
		// Update siblings and first/last child
		if (prevSibling)
		{
			prevSibling.component<Siblings>().next = nextSibling;
		}
		else // First Child
		{
			children.first = siblings.next;
		}

		if (nextSibling)
		{
			nextSibling.component<Siblings>().prev = prevSibling;
		}
		else // Last Child
		{
			children.last = siblings.prev;
		}

		// Remove parent at the end
		child.component<Parent>() = Parent{};
	}

	void Entity::removeChildren()
	{
		AGX_ASSERT_X(*this, "Cannot remove children: Entity is null");
		AGX_ASSERT_X(hasComponent<Children>(), "Cannot remove children: Entity does not have children");

		// This needs to be done in two steps to avoid invalidating the iterator
		auto& children = component<Children>();
		std::vector<Entity> childrenToRemove;
		childrenToRemove.reserve(children.count);
		for (auto child : children)
		{
			childrenToRemove.emplace_back(child);
		}

		children = Children{};
		for (auto& child : childrenToRemove)
		{
			child.component<Parent>() = Parent{};
			child.component<Siblings>() = Siblings{};
		}
	}
}