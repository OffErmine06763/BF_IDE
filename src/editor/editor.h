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
		Editor(Console* console, Compiler* compiler, Runner* runner);
		~Editor();
		virtual void init(GLFWwindow* window);
		virtual void render(GLFWwindow* window);
		virtual ImVec4 getClearColor() {
			return clear_color;
		}

		virtual void output(const std::string& line);
		virtual void output(const char* const line);
		virtual void output(char line);
		virtual void compileError(const std::string& line);
		virtual void compileError(const char* const line);
		virtual void compileError(char c);
		virtual void runtimeError(const std::string& line);
		virtual void runtimeError(const char* const line);
		virtual void runtimeError(char c);

		virtual void setUpProgressBar(std::string& label);
		virtual void setUpProgressBar(const char* label);
		virtual void updateProgressBar(float percentage);
		virtual void removeProgressBar();

		virtual void requestInput();
		virtual uint8_t consumeInput();
		virtual bool inputReceived();
		virtual void setColor(const ImVec4& color);
		virtual void notifyInputReceived();

	private:
		void renderTopBar(ImVec2& windowSize, ImVec2& windowPos);
		void renderFolderTree(ImVec2& windowSize, ImVec2& windowPos);
		void openInEditor(const PathNode& file);
		void renderFolder(const PathNode* path);
		void renderFilesEditor(ImVec2& windowSize, ImVec2& windowPos);
		void renderFile(File& file);
		void renderFileSavePopup();
		void renderConsole(ImVec2& windowSize, ImVec2& windowPos);

	protected:
		Compiler* m_compiler;
		Console* m_console;
		Runner* m_runner;
		std::vector<File> m_openedFiles;
		int64_t m_currFile = -1, m_moveToFile = -1;
		std::vector<uint64_t> m_closeQueue, m_closeQueueSave;
		int64_t m_frame = 0;
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
// TODO: befunge
/* TODO:
*	mock class for Editor (and other classes eventually) to remove:
*		if (m_editor != nullptr) ...
*/
