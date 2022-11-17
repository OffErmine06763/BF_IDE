#include "pch.h"
#include "CppUnitTest.h"

#include "compiler.h"

#include <vector>
#include <string>
#include <fstream>

#define TEST_CASE_DIRECTORY GetDirectoryName(__FILE__)

std::string GetDirectoryName(std::string path) {
	const size_t last_slash_idx = path.rfind('\\');
	if (std::string::npos != last_slash_idx)
	{
		return path.substr(0, last_slash_idx + 1);
	}
	return "";
}

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace UnitTests {
	TEST_CLASS(UnitTestsClass) {
	public:
		TEST_METHOD(TestCompilation) {
			bfide::Compiler compiler;
			compiler.m_compiling = true;

			std::ifstream namesFile(TEST_CASE_DIRECTORY + "bf/compile_tests/names.txt");
			std::ifstream parFile(TEST_CASE_DIRECTORY + "bf/compile_tests/par.txt");

			Assert::AreEqual(true, namesFile.is_open() && parFile.is_open(), L"Faild to open files");

			std::vector<std::string> parenthesis, names;
			std::vector<bool> correctResults;
			int parCount, namesCount;
			parFile >> parCount;
			namesFile >> namesCount;
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
				correctResults.push_back(res);
			}

			std::vector<std::pair<std::vector<std::string>, bool>> tests;
			tests.reserve(namesCount * parCount);

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

			std::string name = "Test", error;
			for (std::pair<std::vector<std::string>, bool>& testCase : tests) {
				for (std::string& str : testCase.first) {
					Logger::WriteMessage((str + '\n').c_str());
				}
				bool res = compiler.parseFile(testCase.first, name, error, false);
				Logger::WriteMessage(std::format("Expected: <{}> - Actual: <{}>\n", testCase.second, res).c_str());
				Assert::AreEqual(testCase.second, res, std::wstring(error.begin(), error.end()).c_str());
			}
			namesFile.close();
			parFile.close();
		} // CAUSE: should not use rfind and similar since there could be more than one import per line
	};
}
