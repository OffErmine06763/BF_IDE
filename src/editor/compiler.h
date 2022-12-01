#pragma once
#include "file.h"
#include "runner.h"

#include <thread>
#include <vector>

namespace UnitTests {
	class UnitTestsClass;
}

namespace bfide {
	enum CompileResult {
		SUCCESS = 0, ERROR, ABORT
	};

	class Editor;

	class Compiler {
		friend UnitTests::UnitTestsClass;
	public:
		~Compiler() {
			if (m_compiling) {
				m_compiling = false;
				if (m_compilerThread.joinable())
					m_compilerThread.join();
			}
		}
		void init(Editor* editor) {
			this->m_editor = editor;
		}

		void compile(File* file);
		void compile(File* file, void (*callback)(void* data, std::string& code), void* data = nullptr);
		void createExe(File* file);

		bool lastCompSucc() { return m_lastCompSucc; }
		std::string getCompiledCode() { return m_code; }
		bool isCopiling() { return m_compiling; }

	private:
		CompileResult parseFile(std::vector<std::string>& fileLines, const std::string& filename, std::string& error, bool recursive = true);
		CompileResult compileFile(std::string& filename, std::string& error);
		bool save();
		bool toExecutable();
		void createCppCode(std::string& dest);

	public:
		static constexpr char extra_symbols[] = { '\t', '{', '}', ' ', '/' };

	private:
		Editor* m_editor = nullptr;

		std::filesystem::path m_mergedPath, m_cppPath, m_compilePath, m_path;
		bool m_lastCompSucc = false, m_compiling = false;
		std::string m_code;
		std::stringstream m_ss;

		std::thread m_compilerThread;

		static constexpr const char* MERGED_FILENAME = "merged.bf", * CPP_FILENAME = "merged.cpp";
		static constexpr const char* TEMPLATE_CPP = "#include <iostream>\n\nint main() {{\nint ind = 0, size = {};\nchar *mem = (char*)malloc(sizeof(char) * size);\nfor (int i = 0; i < size; i++)\nmem[i] = 0;\n\n{}\ndelete[] mem;\n}}\n";
	};
}
