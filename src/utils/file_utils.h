#pragma once

#include <filesystem>
#include <fstream>

namespace Aegix::FileUtils
{
	/// @brief Reads a binary file and returns the contents as a vector of chars
	/// @param filePath Path to the file to read
	/// @return A vector of chars with the contents of the file or an empty vector if the file could not be read
	std::vector<char> readBinaryFile(const std::filesystem::path& filePath)
	{
		std::ifstream file{ filePath, std::ios::ate | std::ios::binary };
		if (!file.is_open())
			return {};

		auto fileSize = static_cast<size_t>(file.tellg());
		std::vector<char> buffer(fileSize);

		file.seekg(0);
		file.read(buffer.data(), fileSize);
		file.close();
		return buffer;
	}
}
