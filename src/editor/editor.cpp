#include "editor.h"
#include <iostream>
#include <GLFW/glfw3.h>
#include <fstream>
#include <filesystem>

namespace bfide {
	const std::string Editor::DATA_FOLDER_KEY = "folder";

	Editor::Editor() {
		m_code = "+[----->+++<]>+.+.[--->+<]>---.+[----->+<]>.++.--.";

		std::ifstream in("data.bfidedata");
		char line[1024];
		while (in.getline(line, 1024).gcount() > 0) {
			std::string linestr(line);
			std::string field, value;
			int openInd = linestr.find('\"'), closeInd = linestr.find('\"', openInd + 1);
			field = linestr.substr(openInd + 1, closeInd - openInd - 1);
			openInd = linestr.find('\"', closeInd + 1);
			closeInd = linestr.find('\"', openInd + 1);
			value = linestr.substr(openInd + 1, closeInd - openInd - 1);

			m_data[field] = value;
		}
		in.close();


		m_isFolderOpen = m_data.find(DATA_FOLDER_KEY) != m_data.end();
		if (m_isFolderOpen) {
			bfide::PathNode node(std::filesystem::path(m_data.find(DATA_FOLDER_KEY)->second));
			m_baseFolder = node;
		}
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
		ImGui::SetNextWindowSize({ windowSize.x, windowSize.y * 0.1f });
		ImGui::SetNextWindowPos({ windowPos.x, windowPos.y });
		ImGui::Begin("Top Bar", new bool, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);
		if (ImGui::Button("show tutorial")) {
			show_example = !show_example;
		}
		ImGui::SameLine();
		ImGui::Button("Compile");
		ImGui::SameLine();
		ImGui::Button("Run");
		ImGui::End();
	}
	void Editor::renderFolderTree(ImVec2& windowSize, ImVec2& windowPos) {
		ImGui::SetNextWindowSize({ windowSize.x * 0.2f, windowSize.y * 0.9f });
		ImGui::SetNextWindowPos({ windowPos.x, windowPos.y + windowSize.y * 0.1f });
		ImGui::SetNextWindowSizeConstraints({ 0.0f, windowSize.y * 0.9f }, { windowSize.x * 0.9f, windowSize.y * 0.9f });
		ImGui::Begin("Folder Explorer", new bool);
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
                if (ImGui::Selectable(file.name.c_str(), new bool, ImGuiSelectableFlags_AllowDoubleClick)) {
                    if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
                        File editorFile = File(file.m_path);
                        m_openedFiles.push_back(editorFile);
                        editorFile.open();
                    }
                }
            }
			ImGui::TreePop();
		}
	}
    void Editor::renderFilesEditor(ImVec2& windowSize, ImVec2& windowPos) {
        ImGui::SetNextWindowSize({ windowSize.x * 0.8f, windowSize.y * 0.9f });
        ImGui::SetNextWindowPos({ windowPos.x + windowSize.x * 0.2f, windowPos.y + windowSize.y * 0.1f });
        ImGui::Begin("Editor", new bool, ImGuiWindowFlags_NoTitleBar);

        if (ImGui::BeginTabBar("##files_tab", ImGuiTabBarFlags_Reorderable)) {
            for (File& file : m_openedFiles) {
                if (!file.isOpen())
                    continue;

                ImGuiTabItemFlags tab_flags = (file.isEdited() ? ImGuiTabItemFlags_UnsavedDocument : 0);
                bool visible = ImGui::BeginTabItem(file.getName().c_str(), file.isOpenRef(), tab_flags);

                // Cancel attempt to close when unsaved add to save queue so we can display a popup.
                if (!file.isOpen() && file.isEdited())
                {
                    file.open();
                }

                if (ImGui::BeginPopupContextItem()) {
                    char buf[256];
                    sprintf_s(buf, "Save %s", file.getName());
                    if (ImGui::MenuItem(buf, "CTRL+S", false, file.isOpen()))
                        file.save();
                    if (ImGui::MenuItem("Close", "CTRL+W", false, file.isOpen()))
                        file.close();
                    ImGui::EndPopup();
                }

                if (visible)
                {
                    ImGui::InputTextMultiline("##", file.getContentRef());
                    ImGui::EndTabItem();
                }
            }

            ImGui::EndTabBar();
        }

        ImGui::End();
    }


    /*
	void Editor::renderFilesTab(ImVec2& windowSize, ImVec2& windowPos) {
        ImGui::SetNextWindowSize({ windowSize.x * 0.8f, windowSize.y * 0.9f });
        ImGui::SetNextWindowPos({ windowPos.x + windowSize.x * 0.2f, windowPos.y + windowSize.y * 0.1f });
        ImGui::Begin("Editor", new bool, ImGuiWindowFlags_NoTitleBar);
		if (ImGui::BeginTabBar("##tabs", ImGuiTabBarFlags_Reorderable))
		{
			// [DEBUG] Stress tests
			//if ((ImGui::GetFrameCount() % 30) == 0) docs[1].Open ^= 1;            // [DEBUG] Automatically show/hide a tab. Test various interactions e.g. dragging with this on.
			//if (ImGui::GetIO().KeyCtrl) ImGui::SetTabItemSelected(docs[1].Name);  // [DEBUG] Test SetTabItemSelected(), probably not very useful as-is anyway..

			// Submit Tabs
			for (int doc_n = 0; doc_n < m_openedFiles.size(); doc_n++)
			{
				File* doc = m_openedFiles[doc_n];
				if (!doc->isOpen())
					continue;

				ImGuiTabItemFlags tab_flags = (doc->isDirty() ? ImGuiTabItemFlags_UnsavedDocument : 0);
				bool visible = ImGui::BeginTabItem(doc->name.c_str(), doc->isOpenRef(), tab_flags);

				// Cancel attempt to close when unsaved add to save queue so we can display a popup.
				if (!doc->isOpen() && doc->isDirty())
				{
                    doc->open();
					doc->queueClose();
				}

                if (ImGui::BeginPopupContextItem()) {
                    char buf[256];
                    sprintf_s(buf, "Save %s", doc->name);
                    if (ImGui::MenuItem(buf, "CTRL+S", false, doc->isOpen()))
                        doc->save();
                    if (ImGui::MenuItem("Close", "CTRL+W", false, doc->isOpen()))
                        doc->queueClose();
                    ImGui::EndPopup();
                }

                if (visible)
				{
                    ImGui::InputTextMultiline("##", &doc->content);
					ImGui::EndTabItem();
				}
			}

			ImGui::EndTabBar();
		}
        ImGui::End();
	}
    */
}
