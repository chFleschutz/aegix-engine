#include "option_manager.h"

namespace VEAI
{
	void OptionManager::update(float deltaSeconds)
	{
		if (m_options.empty())
			return;

		m_options.front()->update(deltaSeconds);

		if (m_options.front()->isFinished())
			removeFirstOption();
	}

	void OptionManager::cancelActive()
	{
		if (m_options.empty())
			return;

		m_options.front()->stop();

		removeFirstOption();
	}

	void OptionManager::removeFirstOption()
	{
		assert(!m_options.empty());

		m_options.pop_front();

		if (!m_options.empty())
			m_options.front()->start();
	}

} // namespace VEAI