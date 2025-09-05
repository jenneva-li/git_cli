#include <string>
#include <algorithm>
#include <sstream>
#include <vector>   

#include "gitTree.h"

std::pair<size_t, GitTreeEntry> GitTree::parse_single_tree(const std::vector<unsigned char>& data) {
    GitTreeEntry entry;
    size_t pos = 0;

    while (pos < data.size() && data[pos] != ' ') {
        entry.mode.push_back(static_cast<char>(data[pos]));
        ++pos;
    }
    if (pos == data.size()) {
        throw std::runtime_error("Invalid tree format: no space found");
    }
    ++pos;

    while (pos < data.size() && data[pos] != '\0') {
        entry.path.push_back(static_cast<char>(data[pos]));
        ++pos;
    }
    if (pos == data.size()) {
        throw std::runtime_error("Invalid tree format: no null terminator found");
    }
    ++pos;

    if (pos + 20 > data.size()) {
        throw std::runtime_error("Invalid tree format: not enough data for SHA");
    }
    entry.sha = std::string(reinterpret_cast<const char*>(&data[pos]), 20);
    pos += 20;

    return {pos, entry};
}

std::vector<GitTreeEntry> GitTree::parse_tree(const std::vector<unsigned char>& data) {
    size_t pos = 0;
    std::vector<GitTreeEntry> entries;
    while (pos < data.size()) {
        auto [new_pos, tree_data] = parse_single_tree(std::vector<unsigned char>(data.begin() + pos, data.end()));
        entries.push_back(tree_data);
        pos = new_pos;
    }
    return entries;
}

std::vector<GitTreeEntry> GitTree::sort_tree_leaf(const std::vector<GitTreeEntry>& entries) const {
    std::vector<GitTreeEntry> sorted_entries = entries;
    std::sort(sorted_entries.begin(), sorted_entries.end(),
        [](const GitTreeEntry& a, const GitTreeEntry& b) {
            auto key = [](const GitTreeEntry& entry) {
                if (entry.mode.rfind("10", 0) == 0) {
                    return entry.path;
                } else {
                    return entry.path + "/";
                }
            };
            return key(a) < key(b);
        });
    return sorted_entries;
}

std::string GitTree::serialize_tree(const std::vector<GitTreeEntry>& entries) const {
    std::string result;
    for (const auto& entry : entries) {
        result.append(entry.mode);
        result.push_back(' ');
        result.append(entry.path);
        result.push_back('\0');
        result.append(entry.sha.data(), entry.sha.size());
    }
    return result;
}

std::string GitTree::serialize() const {
    return serialize_tree(sort_tree_leaf(this->entries));
}

void GitTree::deserialize(const std::string& data) {
    this->content = data;
    this->size = data.size();
    this->entries = parse_tree(std::vector<unsigned char>(data.begin(), data.end()));
}