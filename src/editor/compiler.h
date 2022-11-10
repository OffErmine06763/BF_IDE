#pragma once
#include <string>
#include <windows.h>
#include <vector>

class Compiler {
public:
	Compiler();

	bool compileAndExecute(std::string& filename);
	void execute(std::string& code);

public:
	static const std::string outputFilename;
	static const uint16_t max_size = 30000;
	static const uint8_t RED = 12, WHITE = 7;

private:
	bool findErrors(std::string& filecontent, std::string& filename);
	bool import(std::string& filename, std::stringstream& outputcontent);
	bool handleImports(std::string& filecontent, std::stringstream& outputcontent);
	bool save(std::string& outputcontent);

private:
	std::string m_path;
	HANDLE hConsole;
	std::vector<uint8_t> bytes;
};
