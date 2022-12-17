#pragma once
#include "..\dummies\DummyEditor.h"


namespace bfide {
	class SpyEditor : public DummyEditor {
	public:
		std::string& getLastCompileErrorMessage() {
			return m_LastCompileErrorMessage;
		}

		void compileError(const std::string& line) override {
			m_LastCompileErrorMessage = line;
		}
		void compileError(const char* const line) override {
			compileError(std::string(line));
		}
		void compileError(char c) override {
			compileError(std::string(1, c));
		}

	private:
		std::string m_LastCompileErrorMessage = "f";
	};
}
