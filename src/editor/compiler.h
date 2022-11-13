#pragma once
#include "file.h"
#include "console.h"

#include <string>
#include <vector>
#include <filesystem>

namespace bfide {
    class Compiler {
    public:
        Compiler(Console* console);
        Compiler();

        bool compileAndExecute(File* file);
        bool compile(File* file);
        void executeLastCompiled();

        inline bool lastCompSucc() const { return m_lastCompSucc; }

    public:
        static const uint16_t max_size = 30000;

    private:
        void execute();

        bool findErrors(std::string& filecontent, const std::string& filename);
        bool import(std::string& filename, std::stringstream& outputcontent);
        bool handleImports(std::string& filecontent, std::stringstream& outputcontent);
        bool save();

    private:
        bool m_lastCompSucc = false;
        std::string m_code;
        std::filesystem::path m_mergedPath, m_compilePath;
        Console* m_console;
        std::filesystem::path m_path;
        std::vector<uint8_t> bytes;
    };
}
