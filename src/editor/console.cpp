#include "console.h"

#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"
#include "imgui_stdlib.h"

namespace bfide {
    const ImVec4 Console::RED = { 1.0f, 0.0f, 0.0f, 1.0f };
    const ImVec4 Console::WHITE = { 1.0f, 1.0f, 1.0f, 1.0f };

	void Console::write(const std::string& line) {
		m_text = m_text.append(line);
	}
    void Console::write(const char* const line) {
        m_text = m_text.append(line);
    }
    void Console::write(char c) {
        m_text.push_back(c);
    }

	void Console::render() {
        if (ImGui::Button("Clear"))
            clear();

		ImGui::Text(m_text.c_str());
	}
}
