#pragma once

#include "scene/entity.h"

namespace vre
{
	class ScriptComponentBase
	{
	public:
		virtual ~ScriptComponentBase() {}

		virtual void begin() = 0;
		virtual void update(float deltaSeconds) = 0;
		virtual void end() = 0;

	protected:
		template<typename T>
		T& getComponent()
		{
			return m_Entity.getComponent<T>();
		}

	private:
		Entity m_Entity;
	};

} // namespace vre
