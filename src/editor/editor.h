#pragma once
#include "file.h"
#include "path_node.h"
#include "compiler.h"
#include "console.h"
#include "runner.h"

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
        static void notifyInputReceived() {

        }

	public:
		Editor();
        ~Editor();
        void init(GLFWwindow* window);
        void render(GLFWwindow* window);
        ImVec4 getClearColor() {
			return clear_color;
		}

        void output(const std::string& line) {
            m_console.write(line);
        }
        void output(const char* const line) {
            m_console.write(line);
        }
        void output(char line) {
            m_console.write(line);
        }

        void requestInput() {

        }
        uint8_t consumeInput() {
            return 0;
        }
        bool inputReceived() {
            return true;
        }
        void setColor(const ImVec4& color) {
            m_console.setColor(color);
        }

	public:


	private:
		void renderTopBar(ImVec2& windowSize, ImVec2& windowPos);
		void renderFolderTree(ImVec2& windowSize, ImVec2& windowPos);
        void openInEditor(const PathNode& file);
		void renderFolder(const PathNode* path);
		void renderFilesEditor(ImVec2& windowSize, ImVec2& windowPos);
		void renderFile(File& file);
		void renderFileSavePopup();
        void renderConsole(ImVec2& windowSize, ImVec2& windowPos);

	private:
        Compiler m_compiler;
		Console m_console;
        Runner m_runner;
		std::vector<File> m_openedFiles;
        int m_currFile = -1, m_moveToFile = -1;
		std::vector<int> m_closeQueue, m_closeQueueSave;
		int m_frame = 0;
		PathNode m_baseFolder;
		std::unordered_map<std::string, std::string> m_data;

		bool m_isFolderOpen = false, m_firstOpen = true;

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
