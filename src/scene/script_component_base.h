#pragma once

#include "scene/entity.h"
#include "scene/components.h"

namespace vre
{
	class ScriptComponentBase
	{
	public:
		virtual ~ScriptComponentBase() {}

	protected:
		virtual void begin() {}
		virtual void update(float deltaSeconds) {}
		virtual void end() {}

		template<typename T>
		T& getComponent()
		{
			return m_Entity.getComponent<T>();
		}

	private:
		Entity m_Entity;

		friend class Scene;
	};

} // namespace vre
