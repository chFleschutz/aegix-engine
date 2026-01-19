#pragma once

#include "ai/options/option.h"

#include <deque>
#include <memory>
#include <type_traits>

namespace Aegis::AI
{
	/// @brief Manages options in a deque and updates the currently active option
	class OptionManager
	{
	public:
		OptionManager() = default;
		~OptionManager() = default;

		OptionManager(const OptionManager&) = delete;
		OptionManager& operator=(const OptionManager&) = delete;

		/// @brief Updates the currently active option and starts the next one after it has finished
		void update(float deltaSeconds);

		/// @brief Pushes an option to the back of the queue
		Option& emplaceQueued(std::unique_ptr<Option> option)
		{
			auto& op = m_options.emplace_back(std::move(option));

			if (m_options.size() == 1)
				m_options.front()->start();

			return *op.get();
		}

		/// @brief Pushes an option to the back of the queue
		/// @param option The option to push
		/// @note The option only starts after all other options have finished
		template<typename T, typename... Args>
		T& emplaceQueued(Args&&... initArgs)
		{
			static_assert(std::is_base_of_v<Option, T>, "T must be derived from Option");
			auto& option = m_options.emplace_back(std::make_unique<T>(std::forward<Args>(initArgs)...));

			if (m_options.size() == 1)
				m_options.front()->start();

			return static_cast<T&>(*option);
		}

		Option& emplacePrioritized(std::unique_ptr<Option> option)
		{
			if (!m_options.empty())
				m_options.front()->pause();

			auto& op = m_options.emplace_front(std::move(option));

			m_options.front()->start();
			return *op.get();
		}

		/// @brief Pushes an option to the front of the queue
		/// @param option The option to push
		/// @note The current option will be paused and the new option will be started immediately
		template<typename T, typename... Args>
		T& emplacePrioritized(Args&&... args)
		{
			static_assert(std::is_base_of_v<Option, T>, "T must be derived from Option");
			if (!m_options.empty())
				m_options.front()->pause();

			auto& option = m_options.emplace_front(std::make_unique<T>(std::forward<Args>(args)...));

			m_options.front()->start();
			return static_cast<T&>(*option);
		}

		/// @brief Stops and removes the currently active option and starts the next one
		void cancelActive();

		/// @brief Returns the currently active option
		/// @return The currently active option or nullptr if there is none
		Option* activeOption() const { return m_options.empty() ? nullptr : m_options.front().get(); }

	private:
		/// @brief Removes the first option and starts the next one
		void removeFirstOption();

		std::deque<std::unique_ptr<Option>> m_options;
	};
}
