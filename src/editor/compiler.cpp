#include "compiler.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <stack>

const std::string Compiler::outputFilename = "generated/merged.bf";

Compiler::Compiler() {
	hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

}

bool Compiler::findErrors(std::string& filecontent, std::string& filename) {
	for (int i = 0; i < filecontent.length(); i++) {
		char c = filecontent[i];
		if (c != '.' && c != ',' && c != '[' && c != ']' && c != '+' && c != '-' && c != '<' && c != '>') {
			if (c == '{') {
				i = filecontent.find('}', i);
				continue;
			}

			SetConsoleTextAttribute(hConsole, RED);
			std::cout << "Invalid character '" << c << "' in file '" << filename << "'\n";
			SetConsoleTextAttribute(hConsole, WHITE);
			return true;
		}
	}
	// todo: add valid parenthesis
	return false;
}

bool Compiler::import(std::string& filename, std::stringstream& outputcontent) {
	std::cout << "Importing: " << filename << '\n';

	std::ifstream in(m_path + filename);
	if (!in.is_open()) {
		SetConsoleTextAttribute(hConsole, RED);
		std::cout << "Error opening file: " << m_path + filename << "\n";
		SetConsoleTextAttribute(hConsole, WHITE);
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

bool Compiler::save(std::string& outputcontent) {
	std::ofstream out(outputFilename);
	if (!out.is_open()) {
		SetConsoleTextAttribute(hConsole, RED);
		std::cout << "Error opening output file: " << outputFilename << "\n";
		SetConsoleTextAttribute(hConsole, WHITE);
	}
	out << outputcontent;
	out.close();

	std::cout << "Done\n\n";
	return true;
}

bool Compiler::compileAndExecute(std::string& filename) {
	std::cout << "Compiling: " << filename << '\n';

	this->m_path = filename.substr(0, filename.find_last_of('/') + 1);
	std::ifstream in(filename);
	if (!in.is_open()) {
		SetConsoleTextAttribute(hConsole, RED);
		std::cout << "Error opening file\n";
		SetConsoleTextAttribute(hConsole, WHITE);
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

	if (findErrors(filecontent, filename))
		return false;
	if (!handleImports(filecontent, ss))
		return false;

	std::string code = ss.str();
	if (save(code))
		execute(code);

	return true;
}

void Compiler::execute(std::string& code) { // output.bf in src
	bytes.resize(max_size, 0);

	int ind = 0;
	std::stack<int> openLoopPos;
	for (int i = 0; i < code.size(); i++) {
		char c = code[i];
		int count = 0;

		switch (c) {
		case '+': bytes[ind]++;																	break;
		case '-': bytes[ind]--;																	break;
		case '.': std::cout << bytes[ind];														break;
		case ',': std::cout << "\n$ "; int input; std::cin >> input; bytes[ind] = input;		break;

		case '<':
			if (ind == 0) {
				std::string s(i, ' ');

				SetConsoleTextAttribute(hConsole, RED);
				std::cout << "RUNTIME EXCEPTION: INDEX OUT OF BOUNDS\n" << code << '\n' << s << "^ INDEX < 0\n";
				SetConsoleTextAttribute(hConsole, WHITE);
				return;
			}

			ind--;
			break;
		case '>':
			ind++;
			if (ind == max_size) {
				std::string s(i, ' ');

				SetConsoleTextAttribute(hConsole, RED);
				std::cout << "RUNTIME EXCEPTION: INDEX OUT OF BOUNDS\n" << code << '\n' << s << "^ INDEX > " << max_size << '\n';
				SetConsoleTextAttribute(hConsole, WHITE);
				return;
			}

			break;
		case '[': 
			if (bytes[ind] != 0) {
				openLoopPos.push(i); 
				break;
			}
			count = 0;
			for (int j = i + 1; j < code.size(); j++) {
				if (code[j] == '[') {
					count++;
				}
				else if (code[j] == ']') {
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
	std::cout << '\n';

	bytes.clear();
}
