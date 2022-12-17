#pragma once
#include "runner.h"

namespace bfide {
	class DummyRunner : public Runner {
	public:
		DummyRunner() = default;
		~DummyRunner() { }
		void init(Editor* editor) { }

		void run(std::string code) { }
		void stop() { }

		bool isRunning() { }

		void notifyInputReceived() { }
	};
}
