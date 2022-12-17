#include "pch.h"
#include "CppUnitTest.h"

#include "compiler.h"
#include "console.h"

#include <vector>
#include <string>
#include <fstream>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace UnitTests {
	TEST_CLASS(ConsoleTestsClass) {
public:
	TEST_METHOD(Test_ConsoleOut) {
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
