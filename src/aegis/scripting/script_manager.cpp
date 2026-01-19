#include "pch.h"
#include "script_manager.h"

#include "scripting/script_base.h"

namespace Aegis::Scripting
{
	ScriptManager::~ScriptManager()
	{
		handleNewScripts(); // just in case

		for (auto& script : m_scripts)
		{
			script->end();
		}
	}

	void ScriptManager::addScript(ScriptBase* script)
	{
		m_newScripts.emplace_back(script);
	}

	void ScriptManager::update(float deltaSeconds)
	{
		handleNewScripts();

		for (auto& script : m_scripts)
		{
			script->update(deltaSeconds);
		}
	}

	void ScriptManager::handleNewScripts()
	{
		for (auto& script : m_newScripts)
		{
			script->begin();
			m_scripts.emplace_back(script);
		}
		m_newScripts.clear();
	}
}
