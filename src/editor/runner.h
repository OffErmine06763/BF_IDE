#pragma once
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <iostream>
#include <unordered_map>

namespace bfide {
	class Editor;

	class Runner {
	public:
		Runner() = default;
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

	private:

	public:
		static constexpr uint16_t max_size = 30000;
		static constexpr char symbols[] = { '.', ',', '\'', '[', ']', '>', '<', '+', '-' };

	private:

		Editor* m_editor = nullptr;
		bool m_running = false;
		std::vector<uint8_t> m_memory;
		std::thread m_runnerThread;
		std::mutex m_mutex;
		static std::condition_variable m_cv;
	};
}
