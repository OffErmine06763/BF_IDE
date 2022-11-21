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
			this->m_editor = editor;
		}

		void run(std::string code);
		void stop();

		bool isRunning() {
			return m_running;
		}

		void notifyInputReceived() {
			m_cv.notify_one();
		}

	public:
		static const uint16_t max_size = 30000;


	private:
		Editor* m_editor;
		bool m_running = false;
		std::vector<uint8_t> m_memory;
		std::thread m_runnerThread;
		std::mutex m_mutex;
		static std::condition_variable m_cv;
	};
}
