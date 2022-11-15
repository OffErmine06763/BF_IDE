#pragma once
#include "imgui.h"

#include <string>

namespace bfide {
    class Editor;

	class Console {
	public:
        void init(Editor* editor) {
            this->editor = editor;
        }

		inline std::string& getText() { return m_text; }
		void write(const std::string& line);
		void write(const char* const line);
		void write(char line);
        inline void clear() { m_text = ""; }

        bool inputReceived() { return m_inputReceived; }
        void requestInput() { m_inputRequested = true; }
        char consumeInput() {
            m_inputRequested = false;
            m_inputReceived = false;
            return m_input;
        }

        void setColor(ImVec4 color) {
            
        }
		void render(ImVec2& consoleSize);

	public:
        static const ImVec4 RED, WHITE;

	private:

	private:
        Editor* editor;
		std::string m_text = "";
        bool m_inputRequested = false, m_inputReceived = false;
        char m_input;
	};
}
