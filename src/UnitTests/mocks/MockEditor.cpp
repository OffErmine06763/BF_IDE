#include "pch.h"
#include "MockEditor.h"

namespace bfide {
	MockEditor::MockEditor(Console* mockConsole, Compiler* mockCompiler, Runner* mockRunner)
		: Editor(mockConsole, mockCompiler, mockRunner) {
	}

	MockEditor::~MockEditor() {

	}
}
