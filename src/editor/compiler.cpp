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
				if (!compileFile(fileName, error))
					editor->compileError(error);
				else if (m_compiling) {
					m_code = m_ss.str();
					m_ss.clear();
					m_lastCompSucc = save();
					m_compiling = false;
					m_compilerThread.detach();
					callback(data, m_code);
				}
			});
	}

	bool Compiler::compileFile(std::string& filename, std::string& error) {
		if (!m_compiling)
			return false;

		editor->output("Compiling: " + filename + '\n');

		std::filesystem::path currPath = m_path / filename;
		std::ifstream in(currPath);
		if (!in.is_open()) {
			editor->compileError(std::format("Error opening file: {}\n", currPath.string()));
			return false;
		}

		std::string line;
		std::vector<std::string> fileLines;
		while (!in.eof()) {
			in >> line;
			fileLines.push_back(line);
		}
		in.close();

		if (!parseFile(fileLines, filename, error))
			return false;

		// cache here
		return true;
	}

	bool validExtension(std::string& ext) {
		if (ext == ".bf")
			return true;
		return false;
	}
	bool Compiler::parseFile(std::vector<std::string>& fileLines, const std::string& filename, std::string& error, bool recursive /* = true */) {
		if (!m_compiling)
			return false;

		for (int l = 0; l < fileLines.size(); l++) {
			std::string& line = fileLines[l];
			for (int i = 0; i < line.length(); i++) {
				char c = line[i];
				if (c != '.' && c != ',' && c != '[' && c != ']' && c != '+' && c != '-' && c != '<' && c != '>' && c != '{' && c != '}') {
					error = std::format("Invalid character '{}' in file '{}' ({}:{})\n", c, filename, l, i);
					return false;
				}

				if (c == '{') {
					m_ss << line.substr(0, i);
					bool foundClose = false, foundImport = false;
					int prev_l = l, prev_i = i;
					i++;
					for (true; l < fileLines.size(); l++) {
						std::string& importLine = fileLines[l];
						for (true; i < importLine.length(); i++) {
							char importChar = importLine[i];
							if (importChar == '{') {
								error = std::format("Too many '{{' in file '{}' ({}:{})\n", filename, l, i);
								return false;
							}
							else if (importChar == '}') {
								if (!foundImport) {
									error = std::format("Missing filename in import in file '{}' ({}:{})\n", filename, prev_l, prev_i);
									return false;
								}
								foundClose = true;
								break;
							}
							else if (importChar != ' ') {
								foundImport = true;
								int end = i, dot = std::string::npos;
								for (true; end < importLine.length(); end++) {
									if (importLine[end] == ' ' || importLine[end] == '}')
										break;
									else if (importLine[end] == '.')
										dot = end;
								}

								if (dot == std::string::npos) {
									error = std::format("Import filename requires a valid extension ('.bf') in file '{}' ({}:{})\n", filename, l, i);
									return false;
								}
								std::string extension = importLine.substr(dot, end - dot),
									importName = importLine.substr(i, dot - i);
								if (!validExtension(extension)) {
									error = std::format("Import filename requires a valid extension ('.bf') in file '{}' ({}:{})\n", filename, l, i);
									return false;
								}

								if (recursive && !compileFile(importName.append(extension), error)) {
									return false;
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
							line = importLine.substr(i + 1);
							break;
						}
						i = 0;
					}
					if (!foundClose) {
						error = std::format("Import never closes in file '{}' ({}:{})", filename, prev_l, prev_i);
						return false;
					}
				}
				else if (c == '}') { // when finds '{' automatically goes to the corresponding '}' if present
					error = std::format("Too many '}}' in file '{}' ({}:{})\n", filename, l, i);
					return false;
				}

			}
			m_ss << line;
		}

		return true;
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
