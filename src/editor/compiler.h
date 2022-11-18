#pragma once
#include "file.h"

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

		bool lastCompSucc() { return m_lastCompSucc; }
		std::string getCompiledCode() { return m_code; }
		bool isCopiling() { return m_compiling; }

	private:
		CompileResult parseFile(std::vector<std::string>& fileLines, const std::string& filename, std::string& error, bool recursive = true);
		CompileResult compileFile(std::string& filename, std::string& error);
		bool save();

	private:
		Editor* m_editor = nullptr;

		std::filesystem::path m_mergedPath, m_compilePath, m_path;
		bool m_lastCompSucc = false, m_compiling = false;
		std::string m_code;
		std::stringstream m_ss;

		std::thread m_compilerThread;
	};
}
