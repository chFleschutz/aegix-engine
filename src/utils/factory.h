#pragma once

#include <tuple>

/// @brief Creates a  unique_ptr<T> from the given arguments ONLY when create() is called.
template <typename T>
class Factory
{
public:
	/// @brief Constructs a Factory object with the specified arguments.
	/// @param args The arguments used when creating instances of T with create().
	/// @note All arguments are captured by value.
	template <typename... Args>
	explicit Factory(Args&&... args) : m_createFunction([this, capturedArgs = std::forward_as_tuple(std::forward<Args>(args)...)]()
		mutable { return createInstance(capturedArgs); }) {}

	/// @brief Creates a unique_ptr<T> using the stored arguments.
	/// @return A unique_ptr<T> created with the stored arguments.
	std::unique_ptr<T> create() const
	{
		return m_createFunction();
	}

private:
	// Capute by value
	template <typename... CapturedArgs>
	static std::unique_ptr<T> createInstance(std::tuple<CapturedArgs...>& capturedArgs)
	{
		return std::apply([](CapturedArgs&&... args) {
			return std::make_unique<T>(std::forward<CapturedArgs>(args)...);
			}, capturedArgs);
	}

	std::function<std::unique_ptr<T>()> m_createFunction;
};
