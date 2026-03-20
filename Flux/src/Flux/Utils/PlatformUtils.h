#pragma once


#include <filesystem>
#include <fstream>
#include <vector>
#include <string_view>

namespace Flux::Utils {

	std::vector<char> ReadFile(std::string_view filePath)
	{
        std::ifstream file(std::filesystem::path(filePath), std::ios::ate | std::ios::binary);

        if (!file.is_open())
            throw std::runtime_error(std::format("Failed to open the file: {}", filePath));

        std::size_t fileSize = static_cast<std::size_t>(file.tellg());
        std::vector<char> buffer(fileSize);

        file.seekg(0);
        file.read(buffer.data(), fileSize);
        file.close();

        return buffer;
	}

}