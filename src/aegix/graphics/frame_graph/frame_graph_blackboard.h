#pragma once

#include <any>
#include <typeindex>

namespace Aegix::Graphics
{
	class FrameGraphBlackboard
	{
	public:
		FrameGraphBlackboard() = default;
		FrameGraphBlackboard(const FrameGraphBlackboard&) = default;
		FrameGraphBlackboard(FrameGraphBlackboard&&) = default;
		~FrameGraphBlackboard() = default;

		FrameGraphBlackboard& operator=(const FrameGraphBlackboard&) = default;
		FrameGraphBlackboard& operator=(FrameGraphBlackboard&&) = default;

		template<typename T>
		T& operator+=(T&& value)
		{
			return add<T>(std::forward<T>(value));
		}

		template <typename T, typename... Args>
		T& add(Args&&... args)
		{
			assert(!has<T>() && "Blackboard already contains type T");
			return m_storage[typeid(T)].emplace<T>(std::forward<Args>(args)...);
		}

		template <typename T>
		[[nodiscard]]
		T& get()
		{
			assert(has<T>() && "Blackboard does not contain type T");
			return std::any_cast<T&>(m_storage.at(typeid(T)));
		}

		template <typename T>
		[[nodiscard]]
		const T& get() const
		{
			assert(has<T>() && "Blackboard does not contain type T");
			return std::any_cast<const T&>(m_storage.at(typeid(T)));
		}

		template <typename T>
		[[nodiscard]]
		bool has() const
		{
			return m_storage.contains(typeid(T));
		}

	private:
		std::unordered_map<std::type_index, std::any> m_storage;
	};
}