#pragma once

#include <filesystem>

namespace Aegix::File
{
	/// @brief Reads a binary file and returns the contents as a vector of chars
	/// @param filePath Path to the file to read
	/// @return A vector of chars with the contents of the file or an empty vector if the file could not be read
	std::vector<char> readBinary(const std::filesystem::path& filePath);

	/// @brief Reads a binary file and returns the contents as a vector of chars
	/// @param filePath Path to the file to read
	/// @param size Number of bytes to read
	/// @param offset Byte offset from the beginning of the file
	/// @return A vector with the contents of the file or an empty vector if the file could not be read
	std::vector<char> readBinary(const std::filesystem::path& filePath, size_t size, size_t offset = 0);
}
