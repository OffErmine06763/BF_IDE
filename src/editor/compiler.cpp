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
				editor->output("Compiling: " + file->getName() + '\n');

				std::ifstream in(file->getPath());
				if (!in.is_open()) {
					editor->setColor(Console::RED);
					editor->output("Error opening file\n");
					editor->setColor(Console::WHITE);
					return;
				}

				std::string line, filecontent;
				std::stringstream ss;
				while (!in.eof()) {
					in >> line;
					ss << line;
					line.clear();
				}
				in.close();
				filecontent = ss.str();
				ss.str("");

				if (findInvalidChars(filecontent, file->getName()))
					return;
				if (!handleImports(filecontent, ss))
					return;

				m_code = ss.str();
				if (findErrors())
					return;

				m_lastCompSucc = save();
				m_compiling = false;
                m_compilerThread.detach();
				callback(data, m_code);
			});
	}


	bool Compiler::findInvalidChars(std::string& filecontent, const std::string& filename) {
		for (int i = 0; i < filecontent.length(); i++) {
			char c = filecontent[i];
			if (c != '.' && c != ',' && c != '[' && c != ']' && c != '+' && c != '-' && c != '<' && c != '>') {
				if (c == '{') {
					i = filecontent.find('}', i);
					continue;
				}

				editor->setColor(Console::RED);
				std::string line = "Invalid character '" + c;
				line = line.append("' in file '").append(filename).append("\n");
				editor->output(line);
				editor->setColor(Console::WHITE);
				return true;
			}
		}
		// todo: add valid parenthesis
		return false;
	}
	bool Compiler::findErrors() {
		int count = 0;
		for (int i = 0; i < m_code.size(); i++) {
			char c = m_code[i];

			if (c == '[')
				count++;
			else if (c == ']') {
				count--;
				if (count < 0) {
					editor->setColor(Console::RED);
					std::string line = "Too many closed parenthesis: merged.bf char ";
					char buff[10];
					itoa(i, buff, 10);
					line.append(buff);
					line.push_back('\n');
					editor->output(line);
					editor->setColor(Console::WHITE);
					return true;
				}
			}
		}
		return false;
	}

	bool Compiler::import(std::string& filename, std::stringstream& outputcontent) {
		editor->output("Importing: " + filename + '\n');

		std::filesystem::path currPath = m_path / filename;
		std::ifstream in(currPath);
		if (!in.is_open()) {
			editor->setColor(Console::RED);
			editor->output("Error opening file: " + currPath.string() + "\n");
			editor->setColor(Console::WHITE);
			return false;
		}

		std::string line, filecontent;
		std::stringstream ss;
		while (!in.eof()) {
			in >> line;
			ss << line;
		}
		in.close();
		filecontent = ss.str();
		ss.clear();

		if (findInvalidChars(filecontent, filename))
			return false;
		if (!handleImports(filecontent, outputcontent))
			return false;

		return true;
	}

	bool Compiler::handleImports(std::string& filecontent, std::stringstream& outputcontent) {
		int pos = filecontent.find('{'), end = 0;
		while (pos != std::string::npos) {
			outputcontent << filecontent.substr(end, pos - end);

			end = filecontent.find('}', pos + 1);
			std::string importName = filecontent.substr(pos + 1, end - pos - 1);
			if (!import(importName, outputcontent))
				return false;

			pos = filecontent.find('{', pos + 1);
			end++;
		}
		outputcontent << filecontent.substr(end, filecontent.length());

		return true;
	}

	bool Compiler::save() {
		std::filesystem::create_directories(m_compilePath);
		std::ofstream out(m_mergedPath);
		if (!out.is_open()) {
			editor->setColor(Console::RED);
			editor->output("Error opening output file: " + m_mergedPath.string() + "\n");
			editor->setColor(Console::WHITE);
		}
		out << m_code;
		out.close();

		editor->output("Done\n\n");
		return true;
	}
}

