#pragma once
#include "file.h"
#include <thread>
#include <vector>

namespace UnitTests {}
using namespace UnitTests;

namespace bfide {
	class Editor;

	struct ParseResult_t {
		bool succeeded;
		std::vector<std::string> imports;
		std::string error;
	};

	class Compiler {
	public:
		friend class UnitTests;
		~Compiler() {
			if (m_compiling) {
				m_compiling = false;
				m_compilerThread.join();
			}
		}
		void init(Editor* editor) {
			this->editor = editor;
		}

		void compile(File* file);
		void compile(File* file, void (*callback)(void* data, std::string code), void* data);

		bool lastCompSucc() { return m_lastCompSucc; }
		std::string getCompiledCode() { return m_code; }
		bool isCopiling() { return m_compiling; }

	private:
		bool parseFile(std::vector<std::string>& fileLines, const std::string& filename, std::string* error);
		bool compileFile(std::string& filename, std::string* error);
		bool save();

	private:
		Editor* editor;

		std::filesystem::path m_mergedPath, m_compilePath, m_path;
		bool m_lastCompSucc = false, m_compiling = false;
		std::string m_code;
		std::stringstream m_ss;

		std::thread m_compilerThread;
	};
}
