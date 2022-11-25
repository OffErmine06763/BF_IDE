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

		void initProgBar(std::string& label) {
			initProgBar(label.c_str());
		}
		void initProgBar(const char* label) {
			m_hasProgBar = true;
			m_progBarLabel = label;
			m_progBarPercentage = 0;
		}
		void updateProgBar(float percentage) {
			if (!m_hasProgBar)
				return;
			if (percentage >= 1.0f) {
				m_hasProgBar = false;
				return;
			}
			m_progBarPercentage = percentage;
		}
		void removeProgBar() {
			m_hasProgBar = false;
		}

		void setColor(ImVec4 color) {
			
		}
		void render(ImVec2& consoleSize);

	public:
		static const ImVec4 RED, WHITE;

	private:

	private:
		Editor* m_editor = nullptr;
		std::string m_text, m_progBarLabel;
		float m_progBarPercentage;

		bool m_inputRequested = false, m_inputReceived = false, m_hasProgBar = false;
		char m_input;
		char m_inputBuf[2];
	};
}
