#include "compiler.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <stack>

namespace bfide {
	Compiler::Compiler(Console* console) {
        m_console = console;
	}
    Compiler::Compiler() {
        m_console = nullptr;
    }

	bool Compiler::findErrors(std::string& filecontent, const std::string& filename) {
		for (int i = 0; i < filecontent.length(); i++) {
			char c = filecontent[i];
			if (c != '.' && c != ',' && c != '[' && c != ']' && c != '+' && c != '-' && c != '<' && c != '>') {
				if (c == '{') {
					i = filecontent.find('}', i);
					continue;
				}

				m_console->setColor(Console::RED);
                std::string line = "Invalid character '" + c;
                line = line.append("' in file '").append(filename).append("\n");
				m_console->write(line);
				m_console->setColor(Console::WHITE);
				return true;
			}
		}
		// todo: add valid parenthesis
		return false;
	}

	bool Compiler::import(std::string& filename, std::stringstream& outputcontent) {
		m_console->write("Importing: " + filename + '\n');

        std::filesystem::path currPath = m_path.append(filename);
		std::ifstream in(currPath);
		if (!in.is_open()) {
			m_console->setColor(Console::RED);
			m_console->write("Error opening file: " + currPath.string() + "\n");
			m_console->setColor(Console::WHITE);
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

		if (findErrors(filecontent, filename))
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
			m_console->setColor(Console::RED);
			m_console->write("Error opening output file: " + m_mergedPath.string()  + "\n");
			m_console->setColor(Console::WHITE);
		}
		out << m_code;
		out.close();

		m_console->write("Done\n\n");
		return true;
	}

	bool Compiler::compileAndExecute(File* file) {
        if (compile(file))
		    execute();

		return true;
	}

    // merges and generated executable
    bool Compiler::compile(File* file) {
        m_path = file->getPath();
        m_compilePath = m_path.parent_path() / "generated";
        m_mergedPath = m_compilePath / "generated.bf";

        m_console->write("Compiling: " + file->getName() + '\n');

        std::ifstream in(m_path);
        if (!in.is_open()) {
            m_console->setColor(Console::RED);
            m_console->write("Error opening file\n");
            m_console->setColor(Console::WHITE);
            return false;
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

        if (findErrors(filecontent, file->getName()))
            return false;
        if (!handleImports(filecontent, ss))
            return false;

        m_code = ss.str();
        m_lastCompSucc = save();
        return m_lastCompSucc;
    }

    void Compiler::executeLastCompiled() {
        if (m_lastCompSucc) {
            execute();
        }
        else {
            m_console->write("Compile first\n");
        }
    }

    void Compiler::execute() {
        bytes.resize(max_size, 0);

        int ind = 0;
        std::stack<int> openLoopPos;
        for (int i = 0; i < m_code.size(); i++) {
            char c = m_code[i];
            int count = 0;

            switch (c) {
            case '+': bytes[ind]++;																	break;
            case '-': bytes[ind]--;																	break;
            case '.': m_console->write(bytes[ind]);												break;
            case ',': std::cout << "\n$ "; int input; std::cin >> input; bytes[ind] = input;		break;

            case '<':
                if (ind == 0) {
                    std::string s(i, ' ');

                    m_console->setColor(Console::RED);
                    m_console->write("RUNTIME EXCEPTION: INDEX OUT OF BOUNDS\n" + m_code + '\n' + s + "^ INDEX < 0\n");
                    m_console->setColor(Console::WHITE);
                    return;
                }

                ind--;
                break;
            case '>':
                ind++;
                if (ind == max_size) {
                    std::string s(i, ' ');

                    m_console->setColor(Console::RED);
                    std::string max_size_str = "" + max_size;
                    m_console->write("RUNTIME EXCEPTION: INDEX OUT OF BOUNDS\n" + m_code + '\n' + s + "^ INDEX > " + max_size_str + '\n');
                    m_console->setColor(Console::WHITE);
                    return;
                }

                break;
            case '[':
                if (bytes[ind] != 0) {
                    openLoopPos.push(i);
                    break;
                }
                count = 0;
                for (int j = i + 1; j < m_code.size(); j++) {
                    if (m_code[j] == '[') {
                        count++;
                    }
                    else if (m_code[j] == ']') {
                        if (count == 0) {
                            i = j;
                            break;
                        }
                        count--;
                    }
                }
                break;

            case ']':
                if (bytes[ind] != 0)
                    i = openLoopPos.top();
                else
                    openLoopPos.pop();
                break;
            }
        }
        m_console->write("\n");

        bytes.clear();
    }
}
