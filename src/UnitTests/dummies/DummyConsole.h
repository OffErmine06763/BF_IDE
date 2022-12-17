#pragma once
#include "console.h"

namespace bfide {
	class DummyConsole : public Console {
	public:
		void init(Editor* editor) { }

		void write(const std::string& line) { }
		void write(const char* const line) { }
		void write(char line) { }
		inline void clear() { }

		bool inputReceived() { }
		void requestInput() { }
		char consumeInput() { }

		void initProgBar(std::string& label) { }
		void initProgBar(const char* label) { }
		void updateProgBar(float percentage) { }
		void removeProgBar() { }

		void setColor(ImVec4 color) { }
		void render(ImVec2& consoleSize) { }
	};
}
