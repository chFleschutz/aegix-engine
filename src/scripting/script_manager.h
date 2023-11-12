#pragma once

#include <vector>

namespace VEScripting
{
	class ScriptBase;

	class ScriptManager
	{
	public:
		/// @brief Adds a script to call its virtual functions
		/// @param script The script to add
		void addScript(ScriptBase* script);

		/// @brief Calls the update function of each script
		void update(float deltaSeconds);

		/// @brief Calls the end function of each script
		void runtimeEnd();

	private:
		/// @brief Calls the begin function of each script once
		void handleNewScripts();
		
		std::vector<ScriptBase*> m_scripts;
		std::vector<ScriptBase*> m_newScripts;
	};

} // namespace VEScripting