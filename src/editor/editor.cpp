#include "editor.h"
#include <iostream>
#include <GLFW/glfw3.h>
#include <fstream>
#include <filesystem>

namespace bfide {
	const std::string Editor::DATA_FOLDER_KEY = "folder";
	const std::string Editor::DATA_OPENED_KEY = "opened";

	Editor::Editor() {
		std::ifstream in("data.bfidedata");
		if (in.is_open()) {
			std::string field, value;
			while (in >> field) {
				in >> value >> value;
				/*
				int openInd = linestr.find('\"'), closeInd = linestr.find('\"', openInd + 1);
				field = linestr.substr(openInd + 1, closeInd - openInd - 1);
				openInd = linestr.find('\"', closeInd + 1);
				closeInd = linestr.find('\"', openInd + 1);
				value = linestr.substr(openInd + 1, closeInd - openInd - 1);
				*/

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
	}

	Editor::~Editor() {
		std::ofstream out("data.bfidedata");
		for (const auto& entry : m_data) {
			out << entry.first << " = " << entry.second << '\n';
		}
		out.close();
	}

	void Editor::init(GLFWwindow* window) {
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
		ImGui::SameLine();
		ImGui::Button("Compile");
		ImGui::SameLine();
		ImGui::Button("Run");
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
				renderFolder(&folder);
			}
			for (const PathNode& file : path->files) {
				if (ImGui::Selectable(file.name.c_str(), false, ImGuiSelectableFlags_AllowDoubleClick)) {
					if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
						File editorFile = File(file.m_path);
						editorFile.load();
						editorFile.open();
						m_openedFiles.push_back(editorFile);
					}
				}
			}
			ImGui::TreePop();
		}
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
			for (File& file : m_openedFiles) {
				if (!file.isOpen())
					continue;

				ImGuiTabItemFlags tab_flags = (file.isEdited() ? ImGuiTabItemFlags_UnsavedDocument : 0);
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
						m_closeQueue.push_back(&file);
					}
					else
						file.close();
				}

				if (visible) {
					renderFile(file);
					ImGui::EndTabItem();
				}
			}

			ImGui::EndTabBar();
		}

		if (m_closeQueue.size() > 0)
			renderFileSavePopup();

		ImGui::End();
	}
	void Editor::renderFile(File& file) {
		ImGui::InputTextMultiline("##", file.getContentRef(), { editorSize.x * 0.9f, editorSize.y * 0.9f }, ImGuiInputTextFlags_CallbackEdit, fileInputCallback, &file);
	}
	void Editor::renderFileSavePopup() {
		if (!ImGui::IsPopupOpen("Save?"))
			ImGui::OpenPopup("Save?");
		if (ImGui::BeginPopupModal("Save?", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
			ImGui::Text("Save change to the following items?");
			float item_height = ImGui::GetTextLineHeightWithSpacing();
			if (ImGui::BeginChildFrame(ImGui::GetID("frame"), ImVec2(-FLT_MIN, 6.25f * item_height))) {
				for (File* file : m_closeQueue)
					ImGui::Text(file->getName().c_str());
				ImGui::EndChildFrame();
			}

			ImVec2 button_size(ImGui::GetFontSize() * 7.0f, 0.0f);
			if (ImGui::Button("Yes", button_size)) {
				for (File* file : m_closeQueue) {
					file->save();
					file->close();
				}
				m_closeQueue.clear();
				ImGui::CloseCurrentPopup();
			}
			ImGui::SameLine();
			if (ImGui::Button("No", button_size)) {
				for (File* file : m_closeQueue)
					file->close();
				m_closeQueue.clear();
				ImGui::CloseCurrentPopup();
			}
			ImGui::SameLine();
			if (ImGui::Button("Cancel", button_size)) {
				m_closeQueue.clear();
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

		ImGui::End();
	}
}
