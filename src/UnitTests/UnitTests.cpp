#include "pch.h"
#include "CppUnitTest.h"

#include "compiler.h"
#include "console.h"

#include <vector>
#include <string>
#include <fstream>

#define TEST_CASE_DIRECTORY GetDirectoryName(__FILE__)

std::string GetDirectoryName(std::string path) {
	const size_t last_slash_idx = path.rfind('\\');
	if (std::string::npos != last_slash_idx) {
		return path.substr(0, last_slash_idx + 1);
	}
	return "";
}

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace UnitTests {
	TEST_CLASS(UnitTestsClass) {
	public:
		TEST_METHOD(TestMerge) {
			std::string expected_code = "+.[-]><++--,,,,...";

			bfide::Compiler compiler;
			bfide::File file(std::filesystem::path(TEST_CASE_DIRECTORY + "bf/merge_tests/merge.bf"));
			file.open();
			file.load();
			std::string fileName = file.getName(), error;

			compiler.m_compiling = true;
			compiler.m_path = file.getPath().parent_path();
			compiler.m_compilePath = compiler.m_path / "generated";
			compiler.m_mergedPath = compiler.m_compilePath / "merged.bf";
			compiler.compileFile(fileName, error);

			Assert::AreEqual(expected_code, compiler.m_ss.str());
		}

		TEST_METHOD(TestCompilation) {
			bfide::Compiler compiler;
			compiler.m_compiling = true;

			std::ifstream namesFile(TEST_CASE_DIRECTORY + "bf/compile_tests/names.txt");
			std::ifstream parFile(TEST_CASE_DIRECTORY + "bf/compile_tests/par.txt");

			Assert::AreEqual(true, namesFile.is_open() && parFile.is_open(), L"Faild to open files");

			int parCount, namesCount;
			parFile >> parCount;
			namesFile >> namesCount;

			std::vector<std::string> parenthesis, names;
			std::vector<bfide::CompileResult> correctResults;
			parenthesis.reserve(parCount);
			names.reserve(namesCount);
			correctResults.reserve(namesCount);

			std::string line;

			// read parenthesis
			for (int p = 0; p < parCount; p++) {
				parFile >> line;
				int pos = 0;
				while ((pos = line.find('_', pos)) != std::string::npos) {
					line.replace(pos, 1, " ");
				}
				parenthesis.push_back(line);
			}
			// read filenames
			for (int t = 0; t < namesCount; t++) {
				namesFile >> line;
				names.push_back(line);
				bool res;
				namesFile >> res;
				correctResults.push_back(res ? bfide::CompileResult::SUCCESS : bfide::CompileResult::ERROR);
			}

			std::vector<std::pair<std::vector<std::string>, bfide::CompileResult>> tests;
			tests.reserve(static_cast<size_t>(namesCount) * parCount);

			// creazione test
			for (int p = 0; p < parenthesis.size(); p++) {
				std::string currPar = parenthesis[p];
				for (int n = 0; n < names.size(); n++) {
					std::string& currName = names[n];
					std::string currTestCode = currPar;
					int ind = 0;
					while ((ind = currTestCode.find("tests", ind)) != std::string::npos) 
						currTestCode = currTestCode.replace(ind, 5, currName);

					std::vector<std::string> currTest;
					int pos = currTestCode.length(), prevPos = 0;
					while ((pos = currTestCode.find("\\n", prevPos)) != std::string::npos) {
						line = currTestCode.substr(prevPos, pos - prevPos);
						currTest.push_back(line);
						prevPos = pos + 2;
					}
					line = currTestCode.substr(prevPos, pos - prevPos);
					currTest.push_back(line);
					tests.push_back({ currTest, correctResults[n] });
				}
			}

			// esecuzione test
			std::string name = "Test", error;
			for (std::pair<std::vector<std::string>, bfide::CompileResult>& testCase : tests) {
				for (std::string& str : testCase.first) {
					Logger::WriteMessage((str + '\n').c_str());
				}
				bfide::CompileResult res = compiler.parseFile(testCase.first, name, error, false);
				Logger::WriteMessage(std::format("Expected: <{}> - Actual: <{}>\n",
					(testCase.second == bfide::CompileResult::SUCCESS ? 1 : 0),
					(res == bfide::CompileResult::SUCCESS ? 1 : 0)).c_str());
				Assert::AreEqual((testCase.second == bfide::CompileResult::SUCCESS ? 1 : 0), (res == bfide::CompileResult::SUCCESS ? 1 : 0), std::wstring(error.begin(), error.end()).c_str());
			}
			namesFile.close();
			parFile.close();
		}

		TEST_METHOD(TestConsoleOut) {
			bfide::Console console;
			std::string str = "ciao\nmondo", str2 = "\ntest";
			console.write(str);
			Assert::AreEqual(str, console.m_text);
			console.write(str2);
			Assert::AreEqual(str + str2, console.m_text);
			console.write('c');
			Assert::AreEqual(str + str2 + 'c', console.m_text);
			console.write("ciao");
			Assert::AreEqual(str + str2 + 'c' + "ciao", console.m_text);

			console.clear();
			console.write("test");
			console.write('\0');
			console.write("test");
			Assert::AreEqual("testtest", console.m_text.c_str());
		}
	};
}
