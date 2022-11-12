#pragma once
#include <filesystem>
#include <string>
#include <vector>
#include <set>
#include <map>

namespace bfide {
	class PathNode {
	public:
		PathNode(std::filesystem::path path);
		PathNode();

		void update();
        std::filesystem::path getPath() const { return m_path; }
        std::string getPathStr() const { return m_path.string(); }

	public:
		bool operator==(const PathNode& other) const {
			return other.m_path.string() == m_path.string();
		}
		bool operator<(const PathNode& other) const {
			return m_path.string() < other.m_path.string();
		}
		bool operator>(const PathNode& other) const {
			return m_path.string() > other.m_path.string();
		}
        bool operator<=(const PathNode& other) const {
            return m_path.string() <= other.m_path.string();
        }
        bool operator>=(const PathNode& other) const {
            return m_path.string() >= other.m_path.string();
        }

	public:
		bool is_folder;
		std::string name;
		std::set<PathNode> folders, files;

    private:
		std::filesystem::path m_path;

	};
}
