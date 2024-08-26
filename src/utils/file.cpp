#include "file.h"

#include <fstream>

namespace Aegix::File
{
	std::vector<char> readBinary(const std::filesystem::path& filePath)
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

	std::vector<char> readBinary(const std::filesystem::path& filePath, size_t size, size_t offset)
	{
		std::ifstream file{ filePath, std::ios::binary };
		if (!file.is_open())
			return {};

		file.seekg(offset);
		std::vector<char> buffer(size);
		file.read(buffer.data(), size);
		file.close();

		return buffer;
	}
}