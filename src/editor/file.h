#pragma once
#include <string>
#include <filesystem>

namespace bfide {
    class File {
    public:
        File(std::filesystem::path path);

        void open();
        bool inline isOpen() { return m_open; }
        bool* isOpenRef() { return &m_open; }
        void close();
        bool inline isEdited() { return m_edited; }
        void save();

        void setName(std::string& new_name);
        std::string inline getName() { return m_name; }
        std::string inline getContent() { return m_content; }
        std::string* getContentRef() { return &m_content; }

    public:
        

    private:


    private:
        std::filesystem::path m_path;
        std::string m_name, m_content;
        bool m_open, m_edited;
    };
}
