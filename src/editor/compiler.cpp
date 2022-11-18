#include "compiler.h"
#include "editor.h"

#include <fstream>

namespace bfide {
	void Compiler::compile(File* file) {
		compile(file, [](void* data, std::string code) {}, nullptr);
	}
	void Compiler::compile(File* file, void (*callback)(void* data, std::string code), void* data) {
		if (m_compiling)
			return;
		
		m_lastCompSucc = false;
		m_compiling = true;
		m_path = file->getPath().parent_path();
		m_compilePath = m_path / "generated";
		m_mergedPath = m_compilePath / "merged.bf";

		m_compilerThread = std::thread([=]() {
				editor->output("Compilation started\n");
				std::string fileName = file->getName(), error;

				CompileResult res = compileFile(fileName, error);
				if (res == ERROR) {
					editor->compileError(error);
					m_compiling = false;
					m_ss.str("");
					m_lastCompSucc = false;
					m_compilerThread.detach();
				}
				else if (res == ABORT) {
					m_compiling = false;
					m_ss.str("");
					m_lastCompSucc = false;
				}
				else if (res == SUCCESS) {
					m_code = m_ss.str();
					m_ss.str("");
					m_lastCompSucc = save();
					m_compiling = false;
					m_compilerThread.detach();
					callback(data, m_code);
				}
			});
	}

	CompileResult Compiler::compileFile(std::string& filename, std::string& error) {
		if (!m_compiling)
			return ABORT;

		editor->output("Compiling: " + filename + '\n');

		std::filesystem::path currPath = m_path / filename;
		std::ifstream in(currPath);
		if (!in.is_open()) {
			editor->compileError(std::format("Error opening file: {}\n", currPath.string()));
			return ERROR;
		}

		std::string line;
		std::vector<std::string> fileLines;
		while (!in.eof()) {
			in >> line;
			if (line.length() > 0)
				fileLines.push_back(line);
			line = "";
		}
		in.close();

		CompileResult res = parseFile(fileLines, filename, error);
		if (res == ABORT || res == ERROR)
			return res;

		// cache here
		return SUCCESS;
	}

	bool validExtension(std::string& ext) {
		if (ext == ".bf")
			return true;
		return false;
	}
	CompileResult Compiler::parseFile(std::vector<std::string>& fileLines, const std::string& filename, std::string& error, bool recursive /* = true */) {
		if (!m_compiling)
			return ABORT;

		for (int l = 0; l < fileLines.size(); l++) {
			std::string& line = fileLines[l];
			int lineSubstrStart = 0;
			for (int i = 0; i < line.length(); i++) {
				char c = line[i];
				if (c != '.' && c != ',' && c != '[' && c != ']' && c != '+' && c != '-' && c != '<' && c != '>' && c != '{' && c != '}') {
					error = std::format("Invalid character '{}' in file '{}' ({}:{})\n", c, filename, l, i);
					return ERROR;
				}

				if (c == '{') {
					m_ss << line.substr(lineSubstrStart, i - lineSubstrStart);
					bool foundClose = false, foundImport = false;
					int prev_l = l, prev_i = i;
					i++;
					for (true; l < fileLines.size(); l++) {
						line = fileLines[l];
						lineSubstrStart = i;
						for (true; i < line.length(); i++) {
							char importChar = line[i];
							if (importChar == '{') {
								error = std::format("Too many '{{' in file '{}' ({}:{})\n", filename, l, i);
								return ERROR;
							}
							else if (importChar == '}') {
								if (!foundImport) {
									error = std::format("Missing filename in import in file '{}' ({}:{})\n", filename, prev_l, prev_i);
									return ERROR;
								}
								lineSubstrStart = i + 1;
								foundClose = true;
								break;
							}
							else if (importChar != ' ') {
								foundImport = true;
								int end = i, dot = std::string::npos;
								for (true; end < line.length(); end++) {
									if (line[end] == ' ' || line[end] == '}')
										break;
									else if (line[end] == '.')
										dot = end;
								}

								if (dot == std::string::npos) {
									error = std::format("Import filename requires a valid extension ('.bf') in file '{}' ({}:{})\n", filename, l, i);
									return ERROR;
								}
								std::string extension = line.substr(dot, end - dot),
									importName = line.substr(i, dot - i);
								if (!validExtension(extension)) {
									error = std::format("Import filename requires a valid extension ('.bf') in file '{}' ({}:{})\n", filename, l, i);
									return ERROR;
								}

								if (recursive) {
									CompileResult res = compileFile(importName.append(extension), error);
									if (res == ABORT || res == ERROR) {
										return res;
									}
								}
								i = end - 1;
								/*
								int ind = importLine.rfind('.', importLine.length() - 1 - i);
								if (ind == std::string::npos) {
									error = std::format("Import filename requires a valid extension ('.bf') in file '{}' ({}:{})\n", filename, l, i);
									return false;
								}
								int end = importLine.find_last_of("} ", importLine.length());
								if (end == std::string::npos) // file extension ends at the end of the line, valid
									end = importLine.length();

								std::string extension = importLine.substr(ind, end - ind),
									importName = importLine.substr(i, ind - i);
								if (!validExtension(extension)) {
									error = std::format("Import filename requires a valid extension ('.bf') in file '{}' ({}:{})\n", filename, l, i);
									return false;
								}

								if (recursive && !compileFile(importName.append(extension), error)) {
									return false;
								}
								i = end - 1;
								*/
							}
						}
						if (foundClose) {
							break;
						}
						i = 0;
					}
					if (!foundClose) {
						error = std::format("Import never closes in file '{}' ({}:{})", filename, prev_l, prev_i);
						return ERROR;
					}
				}
				else if (c == '}') { // when finds '{' automatically goes to the corresponding '}' if present
					error = std::format("Too many '}}' in file '{}' ({}:{})\n", filename, l, i);
					return ERROR;
				}
			}
			if (lineSubstrStart < line.length())
				m_ss << line.substr(lineSubstrStart);
		}

		return SUCCESS;
	}

	bool Compiler::save() {
		if (!m_compiling)
			return false;

		std::filesystem::create_directories(m_compilePath);
		std::ofstream out(m_mergedPath);
		if (!out.is_open()) {
			editor->compileError(std::format("Error opening output file: %s\n", m_mergedPath.string()));
			return false;
		}
		out << m_code;
		out.close();

		editor->output("Done\n\n");
		return true;
	}
}

// todo: compiled of every single included file to parse it only once, << the saved file content into the m_ss
