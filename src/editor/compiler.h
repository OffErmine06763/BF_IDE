#pragma once
#include "file.h"
#include <thread>

namespace bfide {
	class Editor;

	class Compiler {
	public:
        ~Compiler() {
            if (m_compiling)
                m_compilerThread.detach();
        }
        void init(Editor* editor) {
            this->editor = editor;
        }

        void compile(File* file);
        void compile(File* file, void (*callback)(void* data, std::string code), void* data);

		bool lastCompSucc() {
			return m_lastCompSucc;
		}
        std::string getCompiledCode() {
            return m_code;
        }
        bool isCopiling() {
            return m_compiling;
        }

    private:
        bool findInvalidChars(std::string& filecontent, const std::string& filename);
        bool findErrors();
        bool import(std::string& filename, std::stringstream& outputcontent);
        bool handleImports(std::string& filecontent, std::stringstream& outputcontent);
        bool save();


	private:
        Editor* editor;

        std::filesystem::path m_mergedPath, m_compilePath, m_path;
        bool m_lastCompSucc = false, m_compiling = false;
        std::string m_code;

        std::thread m_compilerThread;
	};
}
