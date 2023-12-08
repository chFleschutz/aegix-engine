#pragma once

#include "ai/knowledge.h"

#include <string>
#include <unordered_map>
#include <memory>

namespace VEAI
{
	class Blackboard
	{
	public:
		Blackboard() = default;
		Blackboard(const Blackboard&) = delete;
		Blackboard(Blackboard&&) = default;
		~Blackboard() = default;

		/// @brief Returns true if a knowledge with given name exists
		bool exists(const std::string& name) const
		{
			return m_knowledgeBase.find(name) != m_knowledgeBase.end();
		}

		/// @brief Set knowledge by name
		/// @note Knowledge is created with given arguments
		template <typename T, typename... Args>
		void set(const std::string& name, Args&&... args)
		{
			m_knowledgeBase[name] = std::make_unique<T>(std::forward<Args>(args)...);
		}

		/// @brief Get knowledge by name
		/// @note If knowledge is not found, nullptr is returned
		Knowledge* get(const std::string& name)
		{
			auto it = m_knowledgeBase.find(name);
			if (it != m_knowledgeBase.end())
				return it->second.get();

			return nullptr;
		}

		/// @brief Get knowledge by name and cast it to T
		/// @note If knowledge is not found or cast fails, nullptr is returned
		template <typename T>
		T* get(const std::string& name)
		{
			return dynamic_cast<T*>(get(name));
		}

	private:
		std::unordered_map<std::string, std::unique_ptr<Knowledge>> m_knowledgeBase;
	};

} // namespace VEAI
