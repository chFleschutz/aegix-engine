#pragma once

namespace Aegix::Scripting
{
	class ScriptBase;

	class ScriptManager
	{
	public:
		ScriptManager() = default;
		ScriptManager(const ScriptManager&) = delete;
		ScriptManager(ScriptManager&&) = delete;
		~ScriptManager();

		ScriptManager& operator=(const ScriptManager&) = delete;
		ScriptManager& operator=(ScriptManager&&) = delete;

		/// @brief Adds a script to call its virtual functions
		/// @param script The script to add
		void addScript(ScriptBase* script);

		/// @brief Calls the update function of each script
		void update(float deltaSeconds);

	private:
		/// @brief Calls the begin function of each script once
		void handleNewScripts();
		
		std::vector<ScriptBase*> m_scripts;
		std::vector<ScriptBase*> m_newScripts;
	};
}
