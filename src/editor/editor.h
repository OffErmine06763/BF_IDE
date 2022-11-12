#pragma once
#include "console.h"
#include "file.h"
#include "path_node.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"
#include "imgui_stdlib.h"

#include <string>
#include <vector>
#include <unordered_map>

namespace bfide {
	class Editor {
	public:
		Editor();
        ~Editor();
        void init(GLFWwindow* window);
		void render(GLFWwindow* window);
		ImVec4 getClearColor() {
			return clear_color;
		}

	public:


	private:
		void renderTopBar(ImVec2& windowSize, ImVec2& windowPos);
		void renderFolderTree(ImVec2& windowSize, ImVec2& windowPos);
		void renderFolder(const PathNode* path);
		void renderFilesEditor(ImVec2& windowSize, ImVec2& windowPos);
        void renderConsole(ImVec2& windowSize, ImVec2& windowPos);

	private:
		Console m_console;
		std::vector<File> m_openedFiles;
		std::vector<File*> m_closeQueue;
		int m_currFile = -1, m_frame = 0;
		PathNode m_baseFolder;
		std::unordered_map<std::string, std::string> m_data;

		bool m_isFolderOpen = false, m_firstOpen;

        ImVec2 folderTreeSize = { -1, -1 }, prevFolderTreeSize = { -1, -1 };
        ImVec2 editorSize = { -1, -1 }, prevEditorSize = { -1, -1 };
        ImVec2 consoleSize = { -1, -1 }, prevConsoleSize = { -1, -1 };

		bool show_example = false;
		bool show_demo_window = true;
		bool show_another_window = false;
		ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

		static const std::string DATA_FOLDER_KEY, DATA_OPENED_KEY;
		static constexpr int UPDATE_ALL_UI = 100;
	};
}

// +[----->+++<]>+.+.[--->+<]>---.+[----->+<]>.++.--.
// TODO: side by side memory viewer
