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
			editor->output("\nSopping execution\n");
		}
	}

	void Runner::run(std::string code) {
		if (m_running)
			return;
		bytes.resize(max_size, 0);

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
					case '+': bytes[exec_ind]++;					break;
					case '-': bytes[exec_ind]--;					break;
					case '.': editor->output(bytes[exec_ind]);	break;
					case ',':
						editor->output("\n$ ");

						editor->requestInput();
						m_cv.wait(lk, [=] { return editor->inputReceived(); });
						bytes[exec_ind] = editor->consumeInput();
						break;

					case '<':
						if (exec_ind == 0) {
							std::string s(i, ' ');

							editor->setColor(Console::RED);
							editor->output("RUNTIME EXCEPTION: INDEX OUT OF BOUNDS\n" + code + '\n' + s + "^ INDEX < 0\n");
							editor->setColor(Console::WHITE);
							return;
						}

						exec_ind--;
						break;
					case '>':
						exec_ind++;
						if (exec_ind == max_size) {
							std::string s(i, ' ');

							editor->setColor(Console::RED);
							std::string max_size_str = "" + max_size;
							editor->output("RUNTIME EXCEPTION: INDEX OUT OF BOUNDS\n" + code + '\n' + s + "^ INDEX > " + max_size_str + '\n');
							editor->setColor(Console::WHITE);
							return;
						}

						break;
					case '[':
						if (bytes[exec_ind] != 0) {
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
						if (bytes[exec_ind] != 0)
							i = openLoopPos.top();
						else
							openLoopPos.pop();
						break;
					}
				}
				editor->output("\n");

                if (m_running)
    				m_runnerThread.detach();
				m_running = false;
				bytes.clear();
			});
	}
}
