#pragma once
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <iostream>

namespace bfide {
	class Editor;

	class Runner {
	public:
		~Runner() {
			if (m_running) {
                stop();
			}
		}
		void init(Editor* editor) {
			this->editor = editor;
		}

		void run(std::string code);
		void stop();

		bool isRunning() {
			return m_running;
		}

	public:
		static const uint16_t max_size = 30000;

	private:
		
	private:
		Editor* editor;
		bool m_running = false;
		std::vector<uint8_t> bytes;
		std::thread m_runnerThread;
		std::mutex m_mutex;
		static std::condition_variable m_cv;
	};
}
