#include "runner.h"
#include "console.h"
#include "editor.h"

#include <stack>

namespace bfide {
	std::condition_variable Runner::m_cv;

	void Runner::stop() {
		if (m_running) {
			m_running = false;
			m_cv.notify_one();
			m_runnerThread.join();
			m_editor->output("\nStopping execution\n");
		}
	}

	void Runner::run(std::string code) {
		if (m_running)
			return;
		m_memory.resize(max_size, 0);

		m_running = true;
		m_runnerThread = std::thread(
			[=]() {
				int exec_ind = 0;
				std::unique_lock<std::mutex> lk(m_mutex);
				std::stack<int> openLoopPos;
				for (int i = 0; i < code.size() && m_running; i++) {
					char c = code[i];
					int count = 0;

					switch (c) {
					case '+': m_memory[exec_ind]++;					break;
					case '-': m_memory[exec_ind]--;					break;
					case '.': m_editor->output(m_memory[exec_ind]);	break;
					case ',':
						m_editor->requestInput();
						m_cv.wait(lk, [=] { return m_editor->inputReceived(); });
						m_memory[exec_ind] = m_editor->consumeInput();
						break;

					case '<':
						if (exec_ind == 0) {
							std::string s(i, ' ');
							m_editor->runtimeError(std::format("RUNTIME EXCEPTION: INDEX OUT OF BOUNDS\n{}\n{}^ INDEX < 0\n", code, s));
							
							return;
						}

						exec_ind--;
						break;
					case '>':
						exec_ind++;
						if (exec_ind == max_size) {
							std::string s(i, ' ');
							m_editor->runtimeError(std::format("RUNTIME EXCEPTION: INDEX OUT OF BOUNDS\n{}\n{}^ INDEX > {}\n", code, s, max_size));
							return;
						}

						break;
					case '[':
						if (m_memory[exec_ind] != 0) {
							openLoopPos.push(i);
							break;
						}
						count = 0;
						for (int j = i + 1; j < code.size(); j++) {
							if (code[j] == '[') {
								count++;
							}
							else if (code[j] == ']') {
								if (count == 0) {
									i = j;
									break;
								}
								count--;
							}
						}
						break;

					case ']':
						if (m_memory[exec_ind] != 0)
							i = openLoopPos.top();
						else
							openLoopPos.pop();
						break;
					}
				}
				m_editor->output("\n--------\n");

				if (m_running)
					m_runnerThread.detach();
				m_running = false;
				m_memory.clear();
			});
	}
}
