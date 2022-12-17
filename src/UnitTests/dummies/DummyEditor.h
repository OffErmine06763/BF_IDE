#pragma once
#include "editor.h"
#include "DummyCompiler.h"
#include "DummyConsole.h"
#include "DummyRunner.h"


namespace bfide {
	class DummyEditor : public Editor {
	public:
		DummyEditor();

		~DummyEditor();
		void init(GLFWwindow* window) { }
		void render(GLFWwindow* window) { }
		ImVec4 getClearColor() { }

		void output(const std::string& line) { }
		void output(const char* const line) { }
		void output(char line) { }
		void compileError(const std::string& line) { }
		void compileError(const char* const line) { }
		void compileError(char c) { }
		void runtimeError(const std::string& line) { }
		void runtimeError(const char* const line) { }
		void runtimeError(char c) { }

		void setUpProgressBar(std::string& label) { }
		void setUpProgressBar(const char* label) { }
		void updateProgressBar(float percentage) { }
		void removeProgressBar() { }

		void requestInput() { }
		uint8_t consumeInput() { }
		bool inputReceived() { }
		void setColor(const ImVec4& color) { }
		void notifyInputReceived() { }
	};
}
