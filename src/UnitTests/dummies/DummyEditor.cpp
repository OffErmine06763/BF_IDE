#include "pch.h"
#include "DummyEditor.h"

namespace bfide {
	DummyEditor::DummyEditor()
		: Editor(new DummyConsole(), new DummyCompiler(), new DummyRunner()) {
	}

	DummyEditor::~DummyEditor() {
		delete m_console;
		delete m_compiler;
		delete m_runner;
	}
}
