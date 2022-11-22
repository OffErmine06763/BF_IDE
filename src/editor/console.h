#pragma once
#include "imgui.h"

#include <string>
#include <vector>

namespace UnitTests {
	class UnitTestsClass;
}

namespace bfide {
	class Editor;

	class Console {
		friend UnitTests::UnitTestsClass;
	public:
		void init(Editor* editor) {
			this->m_editor = editor;
		}

		void write(const std::string& line);
		void write(const char* const line);
		void write(char line);
		inline void clear() { m_text.clear(); }

		bool inputReceived() { return m_inputReceived; }
		void requestInput();
		char consumeInput();

		void setColor(ImVec4 color) {
			
		}
		void render(ImVec2& consoleSize);

	public:
		static const ImVec4 RED, WHITE;

	private:

	private:
		Editor* m_editor = nullptr;
		std::string m_text;

		bool m_inputRequested = false, m_inputReceived = false;
		char m_input;
		char m_inputBuf[2];
	};
}
