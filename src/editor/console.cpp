#include "console.h"
#include "editor.h"

#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"
#include "imgui_stdlib.h"

#include <iostream>

namespace bfide {
	const ImVec4 Console::RED = { 1.0f, 0.0f, 0.0f, 1.0f };
	const ImVec4 Console::WHITE = { 1.0f, 1.0f, 1.0f, 1.0f };

	void Console::write(const std::string& line) {
		m_text = m_text.append(line);
	}
	void Console::write(const char* const line) {
		write(std::string(line));
	}
	void Console::write(char c) {
		write(std::string(1, c));
	}

	void Console::requestInput() {
		m_inputRequested = true;
		m_inputBuf[0] = 0;
		m_inputBuf[1] = 0;
	}
	char Console::consumeInput() {
		m_inputRequested = false;
		m_inputReceived = false;
		return m_input;
	}

	void Console::render(ImVec2& consoleSize) {
		if (ImGui::Button("Clear"))
			clear();

		ImGui::Text(m_text.c_str());

		if (m_inputRequested) {
			int flags = ImGuiInputTextFlags_EnterReturnsTrue;
			ImGui::Text("$ ");
			ImGui::SameLine();
			if (ImGui::InputText("##", m_inputBuf, 2, flags)) {
				m_inputReceived = true;
				m_inputRequested = false;
				m_input = m_inputBuf[0];
				m_text = m_text + "$ " + m_input;
				m_editor->notifyInputReceived();
			}
		}

		/*
		int flags = (m_inputRequested ? 0 : ImGuiInputTextFlags_ReadOnly) | ImGuiInputTextFlags_CtrlEnterForNewLine | ImGuiInputTextFlags_EnterReturnsTrue;
		if (ImGui::InputTextMultiline("##console", &m_text, { consoleSize.x * 0.9f, consoleSize.y * 0.9f }, flags)) {
			char input;
			bool found = false;
			for (int64_t i = m_text.size() - 1; i >= 0; i--) {
				if (m_text[i] != ' ') {
					if (m_text[i] == '$') {
						break;
					}
					found = true;
					input = m_text[i];
					break;
				}
			}
			if (found) {
				m_inputReceived = true;
				m_inputRequested = false;
				m_input = input;
				m_text.push_back('\n');
				Editor::notifyInputReceived();
			}
			if (!found) {
				m_text.append("\n$ ");
			}
		}
		*/
	}
}
