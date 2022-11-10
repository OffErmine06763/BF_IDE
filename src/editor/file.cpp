#include "file.h"
#include <fstream>
#include <iostream>
#include <sstream>

namespace bfide {
	File::File(std::filesystem::path path) {
		m_path = path.string();
		m_name = path.filename().string();
	}

	void File::open() {
		std::ifstream in(m_path.string());
		std::stringstream ss;
		ss << in.rdbuf();
		m_content = ss.str();
		in.close();

        m_open = true;
	}
    void File::close() {
        m_content = "";
        m_open = false;
    }
    void File::save() {
        std::ofstream out(m_path);
        out << m_content;
        out.close();
        m_edited = false;
    }
}
