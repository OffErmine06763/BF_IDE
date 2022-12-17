#include "compiler.h"
#include "editor.h"

#include <fstream>

namespace bfide {
	void Compiler::createExe(File* file) {
		if (m_compiling)
			return;

		m_lastCompSucc = false;
		m_compiling = true;
		m_path = file->getPath().parent_path();
		m_compilePath = m_path / "generated";
		m_mergedPath = m_compilePath / MERGED_FILENAME;
		m_cppPath = m_compilePath / CPP_FILENAME;

		m_compilerThread = std::thread([=]() {
			m_editor->output("Compilation started\n");
			m_editor->setUpProgressBar("compiling");
			std::string fileName = file->getName(), error;

			CompileResult res = compileFile(fileName, error);
			if (res == ERROR) {
				if (m_editor != nullptr)
					m_editor->compileError(error);
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
				m_editor->setUpProgressBar("creating executable");
				toExecutable();
				m_compiling = false;
				m_compilerThread.detach();
			}

			m_editor->removeProgressBar();
		});
	}

	void Compiler::compile(File* file) {
		compile(file, [](void* data, std::string& code) {}, nullptr);
	}
	void Compiler::compile(File* file, void (*callback)(void* data, std::string& code), void* data) {
		if (m_compiling)
			return;

		m_compilerThread = std::thread([=]() {
			CompileResult res = compileSyncronous(file);
				if (res == ERROR) {
					m_compilerThread.detach();
				}
				else if (res == SUCCESS) {
					m_compilerThread.detach();
					callback(data, m_code);
				}
			});
	}
	CompileResult Compiler::compileSyncronous(File* file) {
		if (m_compiling)
			return RUNNING;

		m_lastCompSucc = false;
		m_compiling = true;
		m_path = file->getPath().parent_path();
		m_compilePath = m_path / "generated";
		m_mergedPath = m_compilePath / "merged.bf";

		m_editor->output("Compilation started\n");
		std::string fileName = file->getName(), error;

		CompileResult res = compileFile(fileName, error);
		if (res == ERROR) {
			m_editor->compileError(error);
			m_compiling = false;
			m_ss.str("");
			m_lastCompSucc = false;
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
		}

		return res;
	}

	CompileResult Compiler::compileFile(std::string& filename, std::string& error) {
		if (!m_compiling)
			return ABORT;

		m_editor->output("Compiling: " + filename + '\n');

		std::filesystem::path currPath = m_path / filename;
		std::ifstream in(currPath);
		if (!in.is_open()) {
			m_editor->compileError(std::format("Error opening file: {}\n", currPath.string()));
			return ERROR;
		}

		std::string line;
		std::vector<std::string> fileLines;
		while (std::getline(in, line)) {
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
	inline bool isInvalidChar(char c) {
		for (char s : Runner::symbols) {
			if (c == s)
				return false;
		}
		for (char s : Compiler::extra_symbols) {
			if (c == s)
				return false;
		}
		return true;
	}
	CompileResult Compiler::parseFile(std::vector<std::string>& fileLines, const std::string& filename, std::string& error, bool recursive /* = true */) {
		if (!m_compiling)
			return ABORT;

		for (size_t l = 0; l < fileLines.size(); l++) {
			std::string& line = fileLines[l];
			size_t lineSubstrStart = 0, lineSubstrEnd = line.length();
			for (size_t i = 0; i < line.length(); i++) {
				char c = line[i];
				if (isInvalidChar(c)) {
					error = std::format("Invalid character '{}' in file '{}' ({}:{})\n", c, filename, l, i);
					return ERROR;
				}

				if (c == '{') {
					m_ss << line.substr(lineSubstrStart, i - lineSubstrStart);
					bool foundClose = false, foundImport = false;
					size_t prev_l = l, prev_i = i;
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
								size_t end = i, dot = std::string::npos;
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
				else if (c == '/') {
					if (i + 1 < line.length() && line[i + 1] == '/') {
						lineSubstrEnd = i;
						break;
					}
					else {
						error = std::format("Invalid comment in file '{}' ({}:{}): expected '//'", filename, l, i);
						return ERROR;
					}
				}
			}
			if (lineSubstrStart < line.length()) {
				std::string sub = line.substr(lineSubstrStart, lineSubstrEnd - lineSubstrStart);
				size_t ind = 0;
				while ((ind = sub.find(' ', ind)) != std::string::npos)
					sub.erase(ind, 1);
				ind = 0;
				while ((ind = sub.find('\t', ind)) != std::string::npos)
					sub.erase(ind, 1);
				if (sub.length() > 0)
					m_ss << sub;
			}
		}

		return SUCCESS;
	}

	bool Compiler::save() {
		if (!m_compiling)
			return false;

		std::filesystem::create_directories(m_compilePath);
		std::ofstream out(m_mergedPath);
		if (!out.is_open()) {
			m_editor->compileError(std::format("Error opening output file: %s\n", m_mergedPath.string()));
			return false;
		}
		out << m_code;
		out.close();

		m_editor->output("Done\n\n");
		return true;
	}
	bool Compiler::toExecutable() {
		// create cpp file;
		if (!m_compiling || !m_lastCompSucc)
			return false;

		m_editor->output("Creating executable\n");
		std::filesystem::create_directories(m_compilePath);
		std::ofstream out(m_cppPath);
		if (!out.is_open()) {
			m_editor->compileError(std::format("Error opening output cpp file: %s\n", m_cppPath.string()));
			return false;
		}
		std::string cpp_code;
		createCppCode(cpp_code);
		out << cpp_code;
		out.close();

		m_editor->output("Done\n\n");
		return true;

		// compile cpp file;
	}
	void Compiler::createCppCode(std::string& dest) {
		dest.clear();
		for (int i = 0; i < m_code.length(); i++) {
			switch (m_code[i])
			{
			case '.': dest = dest.append("std::cout << mem[ind];\n"); break;
			case ',': dest = dest.append("std::cin >> mem[ind];\n"); break;
			case '<': dest = dest.append("ind--;\n"); break;
			case '>': dest = dest.append("ind++;\n"); break;
			case '+': dest = dest.append("mem[ind]++;\n"); break;
			case '-': dest = dest.append("mem[ind]--;\n"); break;
			case '[': dest = dest.append("while (mem[ind] != 0) {\n"); break;
			case ']': dest = dest.append("}\n"); break;
			default:
				break;
			}
			m_editor->updateProgressBar((float)i / m_code.length());
			std::this_thread::sleep_for(std::chrono::duration<long long, std::milli>(25));
		}
		dest = std::format(TEMPLATE_CPP, Runner::max_size, dest);
	}
}

// TODO: compiled of every single included file to parse it only once, << the saved file content into the m_ss
// TODO: prevent recursive includes
