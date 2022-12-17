#include "editor.h"
#include <iostream>
#include <GLFW/glfw3.h>
#include <fstream>
#include <filesystem>
#include <thread>

namespace bfide {
	const std::string Editor::DATA_FOLDER_KEY = "folder";
	const std::string Editor::DATA_OPENED_KEY = "opened";

	Editor::Editor() {
		m_compiler.init(this);
		m_runner.init(this);
		m_console.init(this);
	}
	Editor::~Editor() {
		std::ofstream out("data.bfidedata");
		for (const auto& entry : m_data) {
			out << entry.first << " = " << entry.second << '\n';
		}
		out.close();
	}

	void Editor::init(GLFWwindow* window) {
		std::ifstream in("data.bfidedata");
		if (in.is_open()) {
			std::string field, value;
			while (in >> field) {
				in >> value >> value;

				m_data[field] = value;
			}
			in.close();


			m_isFolderOpen = m_data.find(DATA_FOLDER_KEY) != m_data.end();
			if (m_isFolderOpen) {
				bfide::PathNode node(std::filesystem::path(m_data.find(DATA_FOLDER_KEY)->second));
				m_baseFolder = node;
			}
			m_firstOpen = m_data.find(DATA_OPENED_KEY) == m_data.end();
			if (m_firstOpen) {
				m_data.insert({ DATA_OPENED_KEY, "true" });
			}
		}

		if (m_firstOpen) {
			int windowWidth, windowHeight, windowPosX, windowPosY;
			glfwGetWindowSize(window, &windowWidth, &windowHeight);
			glfwGetWindowPos(window, &windowPosX, &windowPosY);
			folderTreeSize = { windowWidth * 0.2f, windowHeight * 0.9f };
			editorSize = { windowWidth * 0.8f, windowHeight * 0.5f };
			consoleSize = { windowWidth * 0.8f, windowHeight * 0.4f };
		}
	}

	void resetLayout(ImVec2& windowSize, ImVec2& windowPos) {
		ImGui::SetWindowSize({ windowSize.x, windowSize.y * 0.1f });
		ImGui::SetWindowPos({ windowPos.x, windowPos.y });
		ImGui::SetWindowSize("Folder Explorer", { windowSize.x * 0.2f, windowSize.y * 0.9f });
		ImGui::SetWindowPos("Folder Explorer", { windowPos.x, windowPos.y + windowSize.y * 0.1f });
		ImGui::SetWindowSize("Editor", { windowSize.x * 0.8f, windowSize.y * 0.5f });
		ImGui::SetWindowPos("Editor", { windowPos.x + windowSize.x * 0.2f, windowPos.y + windowSize.y * 0.1f });
		ImGui::SetWindowSize("Console", { windowSize.x * 0.8f, windowSize.y * 0.4f });
		ImGui::SetWindowPos("Console", { windowPos.x + windowSize.x * 0.2f, windowPos.y + windowSize.y * 0.6f });
	}

	void Editor::render(GLFWwindow* window) {
		m_frame++;
		if (m_frame == UPDATE_ALL_UI) {
			m_frame = 0;
			m_baseFolder.update();
		}

		int windowWidth, windowHeight, windowPosX, windowPosY;
		glfwGetWindowSize(window, &windowWidth, &windowHeight);
		glfwGetWindowPos(window, &windowPosX, &windowPosY);
		ImVec2 windowSize(windowWidth, windowHeight), windowPos(windowPosX, windowPosY);

		renderTopBar(windowSize, windowPos);
		renderFolderTree(windowSize, windowPos);
		renderFilesEditor(windowSize, windowPos);
		renderConsole(windowSize, windowPos);

		if (show_example) {
			// 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
			if (show_demo_window)
				ImGui::ShowDemoWindow(&show_demo_window);

			// 2. Show a simple window that we create ourselves. We use a Begin/End pair to create a named window.
			{
				static float f = 0.0f;
				static int counter = 0;

				ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

				ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
				ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
				ImGui::Checkbox("Another Window", &show_another_window);

				ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
				ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

				if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
					counter++;
				ImGui::SameLine();
				ImGui::Text("counter = %d", counter);

				ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
				ImGui::End();
			}

			// 3. Show another simple window.
			if (show_another_window)
			{
				ImGui::Begin("Another Window", &show_another_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
				ImGui::Text("Hello from another window!");
				if (ImGui::Button("Close Me"))
					show_another_window = false;
				ImGui::End();
			}
		}
	}

	void Editor::renderTopBar(ImVec2& windowSize, ImVec2& windowPos) {
		ImGui::SetNextWindowSize({ windowSize.x, windowSize.y * 0.1f }, ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowPos({ windowPos.x, windowPos.y }, ImGuiCond_FirstUseEver);
		ImGui::Begin("Top Bar", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
		if (ImGui::Button("Show tutorial")) {
			show_example = !show_example;
		}
		ImGui::SameLine();
		if (ImGui::Button("Reset layout")) {
			resetLayout(windowSize, windowPos);
		}
		if (m_currFile != -1 && !m_runner.isRunning() && !m_compiler.isCopiling()) {
			ImGui::SameLine();
			if (ImGui::Button("Compile")) {
				m_compiler.compile(&m_openedFiles[m_currFile]);
			}
			ImGui::SameLine();
			if (ImGui::Button("Compile & Run")) {
				m_compiler.compile(&m_openedFiles[m_currFile],
					[](void* data, std::string& code) {
						Runner* runner = (Runner*)data;
						runner->run(code);
					}, (void*)&m_runner);
			}
			ImGui::SameLine();
			if (ImGui::Button("Generate Exe")) {
				m_compiler.createExe(&m_openedFiles[m_currFile]);
			}
		}
		if (m_compiler.lastCompSucc() && !m_runner.isRunning()) {
			ImGui::SameLine();
			if (ImGui::Button("Run")) {
				m_runner.run(m_compiler.getCompiledCode());
			}
		}
		if (m_runner.isRunning()) {
			ImGui::SameLine();
			if (ImGui::Button("Stop")) {
				m_runner.stop();
			}
		}
		ImGui::End();
	}

	void Editor::renderFolderTree(ImVec2& windowSize, ImVec2& windowPos) {
		ImGui::SetNextWindowSizeConstraints({ windowSize.x * 0.1f, windowSize.y * 0.9f }, { windowSize.x * 0.9f, windowSize.y * 0.9f });
		ImGui::Begin("Folder Explorer", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove);

		prevFolderTreeSize = folderTreeSize;
		if (prevEditorSize.x != editorSize.x ) {
			ImVec2 size = { windowSize.x - editorSize.x, windowSize.y * 0.9f };
			ImGui::SetWindowSize(size);
			ImGui::SetWindowPos({ windowPos.x, windowPos.y + windowSize.y * 0.1f });
			folderTreeSize = size;
		}
		else if (prevConsoleSize.x != consoleSize.x) {
			ImVec2 size = { windowSize.x - consoleSize.x, windowSize.y * 0.9f };
			ImGui::SetWindowSize(size);
			ImGui::SetWindowPos({ windowPos.x, windowPos.y + windowSize.y * 0.1f });
			folderTreeSize = size;
		}
		else {
			folderTreeSize = ImGui::GetWindowSize();
		}

		ImGui::Text("Folder Explorer");
		if (m_isFolderOpen) {
			renderFolder(&m_baseFolder);
		}
		else {
			ImGui::Button("Open folder");
		}
		ImGui::End();
	}
	void Editor::renderFolder(const PathNode* path) {
		if (ImGui::TreeNode(path->name.c_str())) {
			for (const PathNode& folder : path->folders) {
				if (ImGui::BeginPopupContextItem()) {
					if (ImGui::MenuItem("Rename"))
						((PathNode*)&folder)->rename();
					if (ImGui::MenuItem("Delete"))
						((PathNode*)&folder)->del();
					ImGui::EndPopup();
				}
				renderFolder(&folder);
			}
			for (const PathNode& file : path->files) {
				if (ImGui::BeginPopupContextItem()) {
					if (ImGui::MenuItem("Open"))
						openInEditor(file);
					if (ImGui::MenuItem("Rename"))
						((PathNode*)&file)->rename();
					if (ImGui::MenuItem("Delete"))
						((PathNode*)&file)->del();
					ImGui::EndPopup();
				}
				if (ImGui::Selectable(file.name.c_str(), false, ImGuiSelectableFlags_AllowDoubleClick)) {
					if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
						openInEditor(file);
					}
				}
			}
			ImGui::TreePop();
		}
	}
	void Editor::openInEditor(const PathNode& file) {
		bool present = false;
		for (int ind = 0; ind < m_openedFiles.size(); ind++) {
			if (m_openedFiles[ind].getPathStr() == file.getPathStr()) {
				m_moveToFile = ind;
				present = true;
				break;
			}
		}
		if (present)
			return;

		m_moveToFile = m_openedFiles.size();
		m_openedFiles.push_back(File(file.getPath()));
		m_openedFiles[m_moveToFile].load();
		m_openedFiles[m_moveToFile].open();
	}

	int fileInputCallback(ImGuiInputTextCallbackData* data) {
		File* currfile = (File*)data->UserData;
		currfile->setEdited();
		return 0;
	}
	void Editor::renderFilesEditor(ImVec2& windowSize, ImVec2& windowPos) {
		ImGui::SetNextWindowSizeConstraints({ windowSize.x * 0.1f, windowSize.y * 0.1f }, { windowSize.x * 0.9f, windowSize.y * 0.8f });
		ImGui::Begin("Editor", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove);

		prevEditorSize = editorSize;
		if (folderTreeSize.x != prevFolderTreeSize.x) {
			ImVec2 size = ImGui::GetWindowSize(), pos = {windowPos.x + folderTreeSize.x, windowPos.y + windowSize.y * 0.1f};
			size.x = windowSize.x - folderTreeSize.x;
			ImGui::SetWindowSize(size);
			ImGui::SetWindowPos(pos);
			editorSize = size;
		}
		else if (consoleSize.y != prevConsoleSize.y) {
			ImVec2 size = ImGui::GetWindowSize(), pos = { windowPos.x + folderTreeSize.x, windowPos.y + windowSize.y * 0.1f };
			size.y = windowSize.y * 0.9f - consoleSize.y;
			ImGui::SetWindowSize(size);
			ImGui::SetWindowPos(pos);
			editorSize = size;
		}
		else {
			editorSize = ImGui::GetWindowSize();
		}

		if (ImGui::BeginTabBar("##files_tab", ImGuiTabBarFlags_Reorderable)) {
			for (size_t ind = 0; ind < m_openedFiles.size(); ind++) {
				File& file = m_openedFiles[ind];
				if (!file.isOpen())
					continue;

				ImGuiTabItemFlags tab_flags = (file.isEdited() ? ImGuiTabItemFlags_UnsavedDocument : 0);
				if (ind == m_moveToFile) {
					tab_flags |= ImGuiTabItemFlags_SetSelected;
					m_moveToFile = -1;
				}
				bool visible = ImGui::BeginTabItem(file.getName().c_str(), file.isOpenRef(), tab_flags);

				if (ImGui::BeginPopupContextItem()) {
					if (ImGui::MenuItem("Save", "CTRL+S", false, file.isOpen()))
						file.save();
					if (ImGui::MenuItem("Close", "CTRL+W", false, file.isOpen()))
						file.wantClose();
					ImGui::EndPopup();
				}
				if (ImGui::IsKeyDown(ImGuiKey_LeftCtrl)) {
					if (ImGui::IsKeyPressed(ImGuiKey_S, false))
						file.save();
					if (ImGui::IsKeyPressed(ImGuiKey_W, false))
						file.wantClose();
				}

				// Cancel attempt to close when unsaved add to save queue so we can display a popup.
				if (!file.isOpen() || file.toClose()) {
					if (file.isEdited()) {
						file.open();
						m_closeQueueSave.push_back(ind);
					}
					else {
						file.close();
						m_closeQueue.push_back(ind);
					}
				}

				if (visible) {
					m_currFile = ind;
					renderFile(file);
					ImGui::EndTabItem();
				}
			}

			ImGui::EndTabBar();
		}

		if (m_closeQueueSave.size() > 0)
			renderFileSavePopup();

		for (uint64_t ind : m_closeQueue) {
			m_openedFiles[ind].close();
			m_openedFiles.erase(m_openedFiles.begin() + ind);
			if (ind == m_currFile) {
				m_currFile = -1;
			}
		}
		m_closeQueue.clear();

		ImGui::End();
	}
	void Editor::renderFile(File& file) {
		ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 255, 0, 255));
		ImGui::InputTextMultiline("##", file.getContentRef(), { editorSize.x * 0.9f, editorSize.y * 0.9f }, ImGuiInputTextFlags_CallbackEdit, fileInputCallback, &file);
		ImGui::PopStyleColor();
	}
	void Editor::renderFileSavePopup() {
		if (!ImGui::IsPopupOpen("Save?"))
			ImGui::OpenPopup("Save?");
		if (ImGui::BeginPopupModal("Save?", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
			ImGui::Text("Save change to the following items?");
			float item_height = ImGui::GetTextLineHeightWithSpacing();
			if (ImGui::BeginChildFrame(ImGui::GetID("frame"), ImVec2(-FLT_MIN, 6.25f * item_height))) {
				for (uint64_t ind : m_closeQueueSave)
					ImGui::Text(m_openedFiles[ind].getName().c_str());
				ImGui::EndChildFrame();
			}

			ImVec2 button_size(ImGui::GetFontSize() * 7.0f, 0.0f);
			if (ImGui::Button("Yes", button_size)) {
				for (uint64_t ind : m_closeQueueSave) {
					m_openedFiles[ind].save();
					m_openedFiles[ind].close();
					m_openedFiles.erase(m_openedFiles.begin() + ind);
					if (ind == m_currFile)
						m_currFile = -1;
				}
				m_closeQueueSave.clear();
				ImGui::CloseCurrentPopup();
			}
			ImGui::SameLine();
			if (ImGui::Button("No", button_size)) {
				for (uint64_t ind : m_closeQueueSave) {
					m_openedFiles[ind].close();
					m_openedFiles.erase(m_openedFiles.begin() + ind);
					if (ind == m_currFile)
						m_currFile = -1;
				}
				m_closeQueueSave.clear();
				ImGui::CloseCurrentPopup();
			}
			ImGui::SameLine();
			if (ImGui::Button("Cancel", button_size)) {
				m_closeQueueSave.clear();
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}
	}

	void Editor::renderConsole(ImVec2& windowSize, ImVec2& windowPos) {
		ImGui::SetNextWindowSizeConstraints({ windowSize.x * 0.1f, windowSize.y * 0.1f }, { windowSize.x * 0.9f, windowSize.y * 0.8f });
		ImGui::Begin("Console", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove);

		if (folderTreeSize.x != prevFolderTreeSize.x || editorSize.y != prevEditorSize.y || consoleSize.x != prevConsoleSize.x || consoleSize.y != prevConsoleSize.y) {
			prevConsoleSize = consoleSize;
			ImVec2 size = { windowSize.x - folderTreeSize.x, windowSize.y * 0.9f - editorSize.y }, pos = { windowPos.x + folderTreeSize.x, windowPos.y + editorSize.y + windowSize.y * 0.1f };
			ImGui::SetWindowSize(size);
			ImGui::SetWindowPos(pos);
			consoleSize = size;
		}
		else {
			prevConsoleSize = consoleSize;
			consoleSize = ImGui::GetWindowSize();
		}

		m_console.render(consoleSize);

		ImGui::End();
	}
}
