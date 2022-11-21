#include "path_node.h"
#include <iostream>
#include <unordered_set>

namespace bfide {
	PathNode::PathNode(std::filesystem::path path, PathNode* parent /* = nullptr */) {
		m_parent = parent;
		m_path = path;
		name = path.string();
		int64_t pos = -1;
		while ((pos = name.find('/', pos + 1)) != std::string::npos) {
			name.replace(pos, 1, "\\");
		}
		name = name.substr(name.find_last_of('\\') + 1, name.length());
		is_folder = std::filesystem::is_directory(path);

		if (is_folder) {
			for (const std::filesystem::directory_entry& entry : std::filesystem::directory_iterator(path)) {
				if (entry.is_regular_file())
					files.insert(PathNode(entry.path()));
				else
					folders.insert(PathNode(entry.path()));
			}
		}
	}

	// default values never to be displayed
	PathNode::PathNode()
		: name("Error"), is_folder(false) {}

	// TODO: listener for folder changes?
	void PathNode::update() {
		files.clear();
		folders.clear();

		for (const std::filesystem::directory_entry& entry : std::filesystem::directory_iterator(m_path)) {
			if (entry.is_regular_file())
				files.insert(PathNode(entry.path(), this));
			else
				folders.insert(PathNode(entry.path(), this));
		}
	}

	void PathNode::rename() {

	}

	void PathNode::del() {
		std::filesystem::remove(m_path);
		m_parent->deleteChildren(this);
	}
	void PathNode::deleteChildren(PathNode* children) {
		std::set<PathNode>::iterator node, end;
		if (children->is_folder) {
			node = folders.begin();
			end = folders.end();
		}
		else {
			node = files.begin();
			end = files.end();
		}
		for (true; node != folders.end(); node++) {
			if (node->getPathStr() == children->getPathStr()) {
				folders.erase(node);
				return;
			}
		}
	}
}
