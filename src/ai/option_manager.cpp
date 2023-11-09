#include "option_manager.h"

namespace VEAI
{
	void OptionManager::update(float deltaSeconds)
	{
		if (m_options.empty())
			return;

		m_options.front()->update(deltaSeconds);

		if (m_options.front()->isFinished())
			cancelFirst();
	}

	void OptionManager::cancelFirst()
	{
		if (m_options.empty())
			return;

		m_options.front()->stop();
		m_options.pop_front();

		if (!m_options.empty())
			m_options.front()->start();
	}

} // namespace VEAI