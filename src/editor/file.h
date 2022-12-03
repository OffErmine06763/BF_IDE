#pragma once
#include <string>
#include <filesystem>

namespace bfide {
	class File {
	public:
		File(std::filesystem::path path);

		bool load();
		void open();
		bool inline isOpen() { return m_open; }
		bool* isOpenRef() { return &m_open; }
		void close();
		void wantClose() { m_wantClose = true; }
		bool toClose() { return m_wantClose; }
		bool inline isEdited() { return m_edited; }
		void save();

		void setEdited() { m_edited = true; }
		void setName(std::string& new_name);
		std::string inline getName() const { return m_name; }
		std::string inline getContent() const { return m_content; }
		std::string* getContentRef() { return &m_content; }
		std::string getPathStr() const { return m_path.string(); }
		std::filesystem::path& getPath() { return m_path; }

	private:
		std::filesystem::path m_path;
		std::string m_name, m_content;
		bool m_open = false, m_edited = false, m_wantClose = false;
	};
}
