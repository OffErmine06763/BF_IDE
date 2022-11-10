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
		std::filesystem::path m_path;
		std::string name;
		std::set<PathNode> folders, files;
	};
}
