#pragma once

#include "scripting/script_base.h"

#include <iostream>

namespace VEScripting
{
	class TestScript : public ScriptBase
	{
	public:
		/// @brief Initialize member variables here.
		/// @note Don't access protected methods or other components here, they are not initialized yet.
		TestScript(int x)
			: m_x(x)
		{
			std::cout << "TestScript::TestScript(" << m_x << ")" << std::endl;
		}

		/// @brief Acces protected methods and other components here.
		virtual void begin() override
		{
			auto& name = getComponent<VEComponent::Name>();
			std::cout << "TestScript::begin() " << name.name << " " << m_x << std::endl;
		}

		/// @brief Update the script each frame here.
		/// @param deltaSeconds Time between last two frames in seconds.
		virtual void update(float deltaSeconds) override
		{
			std::cout << "TestScript::update(" << deltaSeconds << ")" << std::endl;
		}

		/// @brief Clean up here.
		virtual void end() override
		{
			std::cout << "TestScript::end()" << std::endl;
		}

	private:
		int m_x;
	};

} // namespace VEScripting
