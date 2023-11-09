#pragma once

#include "ai/options/option.h"

#include <memory>
#include <deque>

namespace VEAI
{
	/// @brief Manages options in a deque and updates the currently active option
	class OptionManager
	{
	public:
		OptionManager() = default;
		~OptionManager() = default;

		/// @brief Updates the current option 
		void update(float deltaSeconds);

		/// @brief Pushes an option to the back of the queue
		/// @param option The option to push
		/// @note The option only starts after all other options have finished
		template<typename T, typename... Args>
		void emplaceQueued(Args&&... args)
		{
			m_options.emplace_back(std::make_unique<T>(std::forward<Args>(args)...));

			if (m_options.size() == 1)
				m_options.front()->start();
		}

		/// @brief Pushes an option to the front of the queue
		/// @param option The option to push
		/// @note The current option will be paused and the new option will be started immediately
		template<typename T, typename... Args>
		void emplacePrioritized(Args&&... args)
		{
			if (!m_options.empty())
				m_options.front()->pause();

			m_options.emplace_front(std::make_unique<T>(std::forward<Args>(args)...));
			m_options.front()->start();
		}

		/// @brief Cancels the currently active option and starts the next one
		void cancelFirst();

		/// @brief Returns the currently active option
		Option* currentOption() const { return m_options.empty() ? nullptr : m_options.front().get(); }

	private:
		std::deque<std::unique_ptr<Option>> m_options;
	};

} // namespace VEAI