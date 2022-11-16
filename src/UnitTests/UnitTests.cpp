#include "pch.h"
#include "CppUnitTest.h"
#include "..\editor\compiler.h"
#include <vector>
#include <string>
#include <fstream>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace UnitTests {
	TEST_CLASS(UnitTests) {
	public:
		
		TEST_METHOD(TestCompilation) {
			bfide::Compiler compiler;
			std::ifstream names("bf/compile_tests/names.txt");
			std::ifstream par("bf/compile_tests/par.txt");
			std::string name = "Test", error;

			while (!names.eof()) {
				std::string line;
				std::vector<std::string> lines;

				names >> line;
				lines.push_back(line);
				while (line[line.length() - 1] == '\\') {
					names >> line;
					lines.push_back(line);
				}
				bool correct;
				names >> correct;

				bool result = compiler.parseFile(lines, name, &error);
				Assert::AreEqual(correct, result, std::wstring(error.begin(), error.end()).c_str());
			}
		}
	};
}
