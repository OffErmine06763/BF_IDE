#include "pch.h"
#include "CppUnitTest.h"

#include <vector>
#include <string>
#include <fstream>

#include "compiler.h"
#include "mocks/MockEditor.h"
#include "dummies/DummyEditor.h"
#include "spies/SpyEditor.h"


#ifndef TEST_CASE_DIRECTORY
#define TEST_CASE_DIRECTORY GetDirectoryName(__FILE__)

std::string GetDirectoryName(std::string path) {
	const size_t last_slash_idx = path.rfind('\\');
	if (std::string::npos != last_slash_idx) {
		return path.substr(0, last_slash_idx + 1);
	}
	return "";
}
#endif // !TEST_CASE_DIRECTORY

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace bfide;

namespace UnitTests {
	TEST_CLASS(CompilerTestsClass) {
	public:
		TEST_METHOD(Test_Compilation) {
			Compiler compiler;
			DummyEditor dummyEditor;

			compiler.init(&dummyEditor);
			File file(std::filesystem::path(TEST_CASE_DIRECTORY + "bf/merge_tests/merge.bf"));
			file.open();
			Assert::IsTrue(file.load(), L"Failed to load .bf file");

			compiler.compileSyncronous(&file);

			std::string expected_code = "+.[-]><++--,,,,...";
			Assert::AreEqual(expected_code, compiler.getCompiledCode());
		}

		TEST_METHOD(Test_RecursiveCompilation) {
			Compiler compiler;
			SpyEditor spyEditor;

			compiler.init(&spyEditor);
			File file(std::filesystem::path(TEST_CASE_DIRECTORY + "bf/recursive_tests/rec1.bf"));
			file.open();
			Assert::IsTrue(file.load(), L"Failed to load .bf file");

			CompileResult res = compiler.compileSyncronous(&file);
			Assert::AreEqual((int)CompileResult::ERROR, (int)res);
			Assert::AreEqual(
				std::format(Compiler::ERR_MSG_RECURSIVE_COMPILATION, file.getName()),
				spyEditor.getLastCompileErrorMessage());
		}

		TEST_METHOD(Test_IncludesValidation) {
			Compiler compiler;
			DummyEditor dummyEditor;

			compiler.init(&dummyEditor);
			compiler.m_compiling = true;

			std::ifstream namesFile(TEST_CASE_DIRECTORY + "bf/compile_tests/names.txt");
			std::ifstream parFile(TEST_CASE_DIRECTORY + "bf/compile_tests/par.txt");

			Assert::AreEqual(true, namesFile.is_open() && parFile.is_open(), L"Faild to open files");

			int parCount, namesCount;
			parFile >> parCount;
			namesFile >> namesCount;

			std::vector<std::string> parenthesis, names;
			std::vector<CompileResult> correctResults;
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
				correctResults.push_back(res ? SUCCESS : ERROR);
			}

			std::vector<std::pair<std::vector<std::string>, CompileResult>> tests;
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
			for (std::pair<std::vector<std::string>, CompileResult>& testCase : tests) {
				for (std::string& str : testCase.first) {
					Logger::WriteMessage((str + '\n').c_str());
				}
				CompileResult res = compiler.parseFile(testCase.first, name, error, false);
				Logger::WriteMessage(std::format("Expected: <{}> - Actual: <{}>\n",
					(testCase.second == SUCCESS ? 1 : 0),
					(res == SUCCESS ? 1 : 0)).c_str());
				Assert::AreEqual(
					(testCase.second == SUCCESS ? 1 : 0),
					(res == SUCCESS ? 1 : 0),
					std::wstring(error.begin(), error.end()).c_str());
			}
			namesFile.close();
			parFile.close();
		}
	};
}
