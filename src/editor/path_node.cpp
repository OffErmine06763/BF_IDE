#include "path_node.h"
#include <iostream>
#include <unordered_set>

namespace bfide {
	PathNode::PathNode(std::filesystem::path path) {
		m_path = path;
		name = path.string();
		int pos = -1;
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
				/*
                if (entry.is_regular_file())
					files.insert({ entry.path().string(), PathNode(entry.path()) });
				else
					folders.insert({ entry.path().string(), PathNode(entry.path()) });
                */
			}
		}
	}

	// default values should never be used/displayed
	PathNode::PathNode()
		: name("Error"), is_folder(false) {}

	// TODO: listener for folder changes?
	void PathNode::update() {
        files.clear();
        folders.clear();

        for (const std::filesystem::directory_entry& entry : std::filesystem::directory_iterator(m_path)) {
            if (entry.is_regular_file())
                files.insert(PathNode(entry.path()));
            else
                folders.insert(PathNode(entry.path()));
        }

        /*
		std::unordered_set<std::string> new_files, new_folders;

		for (const std::filesystem::directory_entry& entry : std::filesystem::directory_iterator(m_path)) {
			std::filesystem::path curr_path = entry.path();
			std::string curr_path_str = curr_path.string();

			if (entry.is_regular_file()) {
                auto it = files.find(curr_path_str);
                if (it != files.end()) { // update already indexed folders
                    it->second.update();
                    continue;
                }

				new_files.insert(curr_path_str);
			}
			else {
				// if (!next.contains(curr_path_str))
				new_folders.insert(curr_path_str);
			}
		}
        */
	}
}
