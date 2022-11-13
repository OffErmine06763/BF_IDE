#pragma once
#include <string>
#include "imgui.h"

namespace bfide {
	class Console {
	public:
		inline std::string& getText() { return m_text; }
		void write(const std::string& line);
		void write(const char* const line);
		void write(char line);
        inline void clear() { m_text = ""; }

        void setColor(ImVec4 color) { }

		void render();

	public:
        static const ImVec4 RED, WHITE;

	private:


	private:
		std::string m_text;

	};
}
