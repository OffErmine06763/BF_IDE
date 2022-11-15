#pragma once
#include "file.h"
#include "console.h"

#include <string>
#include <vector>
#include <filesystem>
#include <thread>
#include <iostream>

namespace bfide {
	class dep_Compiler {
	public:
		~dep_Compiler();
		void init(Console* console);

		bool compileAndExecute(File* file);
		bool compile(File* file);
		void executeLastCompiled();

		inline bool lastCompSucc() const { return m_lastCompSucc; }
		inline bool isRunning() const { return m_running; }
		void stop() {
			m_runnerThread.detach();
			m_running = false;
			bytes.clear();
			m_console->write("STOPPING\n");
		}

		static void notifyInputReceived() {
			cv.notify_one();
		}

	public:
		static const uint16_t max_size = 30000;
		std::mutex m;
		static std::condition_variable cv;

	private:
		void execute();

		bool findInvalidChars(std::string& filecontent, const std::string& filename);
		bool findErrors();
		bool import(std::string& filename, std::stringstream& outputcontent);
		bool handleImports(std::string& filecontent, std::stringstream& outputcontent);
		bool save();

	private:
		std::string m_code;
		std::thread m_runnerThread;
		int exec_ind = 0;
		bool m_lastCompSucc = false, m_running = false;

		std::filesystem::path m_mergedPath, m_compilePath;
		Console* m_console = nullptr;
		std::filesystem::path m_path;
		std::vector<uint8_t> bytes;
	};
}
