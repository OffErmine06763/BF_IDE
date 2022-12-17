#pragma once
#include "compiler.h"

namespace bfide {
	class DummyCompiler : public Compiler {
	public:
		~DummyCompiler() { }
		void init(Editor* editor) { }

		void compile(File* file) { }
		void compile(File* file, void (*callback)(void* data, std::string& code), void* data = nullptr) { }
		CompileResult compileSyncronous(File* file) { }
		void createExe(File* file) { }

		bool lastCompSucc() { }
		std::string getCompiledCode() { }
		bool isCopiling() { }
	};
}
