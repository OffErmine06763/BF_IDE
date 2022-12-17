#pragma once
#include "editor.h"
#include "MockCompiler.h"
#include "MockConsole.h"
#include "MockRunner.h"


namespace bfide {
	class MockEditor : public Editor {
	public:
		MockEditor(Console* mockConsole = nullptr, Compiler* mockCompiler = nullptr, Runner* mockRunner = nullptr);

		~MockEditor();
		void init(GLFWwindow* window) override { }
		void render(GLFWwindow* window) override { }
		ImVec4 getClearColor() override { return ImVec4{ 0, 0, 0, 0 }; }

		void output(const std::string& line) override { }
		void output(const char* const line) override { }
		void output(char line) override { }
		void compileError(const std::string& line) override { }
		void compileError(const char* const line) override { }
		void compileError(char c) override { }
		void runtimeError(const std::string& line) override { }
		void runtimeError(const char* const line) override { }
		void runtimeError(char c) override { }

		void setUpProgressBar(std::string& label) override { }
		void setUpProgressBar(const char* label) override { }
		void updateProgressBar(float percentage) override { }
		void removeProgressBar() override { }

		void requestInput() override { }
		uint8_t consumeInput() override { return 0; }
		bool inputReceived() override { return false; }
		void setColor(const ImVec4& color) override { }
		void notifyInputReceived() override { }
	};
}
