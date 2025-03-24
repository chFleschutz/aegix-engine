#pragma once

#include "scripting/script_base.h"

namespace Aegix::Scripting
{
	class TestScript : public ScriptBase
	{
	public:
		/// @brief Initialize member variables here.
		/// @note Don't access protected methods or other components here, they are not initialized yet.
		TestScript(int x)
			: m_x(x)
		{
			ALOG::info("TestScript::TestScript({})", m_x);
		}

		/// @brief Acces protected methods and other components here.
		virtual void begin() override
		{
			ALOG::info("TestScript::begin({})", component<Name>().name);
		}

		/// @brief Update the script each frame here.
		/// @param deltaSeconds Time between last two frames in seconds.
		virtual void update(float deltaSeconds) override
		{
			ALOG::info("TestScript::update({})", deltaSeconds);
		}

		/// @brief Clean up here.
		virtual void end() override
		{
			ALOG::info("TestScript::end()");
		}

	private:
		int m_x;
	};
}
