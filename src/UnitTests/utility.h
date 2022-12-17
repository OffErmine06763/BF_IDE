#pragma once
#include <string>

std::string GetDirectoryName(std::string path) {
	const size_t last_slash_idx = path.rfind('\\');
	if (std::string::npos != last_slash_idx) {
		return path.substr(0, last_slash_idx + 1);
	}
	return "";
}
