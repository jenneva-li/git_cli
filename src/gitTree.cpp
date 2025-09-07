#include <string>
#include <algorithm>
#include <sstream>
#include <vector>   
#include <array>

#include "gitTree.h"
#include "gitBlob.h"

std::pair<size_t, GitTreeEntry> GitTree::parse_single_tree(const std::vector<unsigned char>& data, size_t pos) {
    GitTreeEntry entry;

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
    std::ostringstream hexsha;
    for (int i = 0; i < 20; ++i)
    {
        hexsha << std::hex << std::setw(2) << std::setfill('0')
               << static_cast<int>(data[pos + i]);
    }
    entry.sha = hexsha.str();
    pos += 20;

    return {pos, entry};
}

std::vector<GitTreeEntry> GitTree::parse_tree(const std::vector<unsigned char>& data) {
    size_t pos = 0;
    std::vector<GitTreeEntry> entries;
    while (pos < data.size()) {
        auto [new_pos, tree_data] = parse_single_tree(data, pos);
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
        result.append(reinterpret_cast<const char*>(entry.sha.data()), entry.sha.size());
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

void GitTree::recursive_ls_tree(const GitRepository& repo, const std::string& tree_sha, const std::string& prefix) {
    auto obj = read_object(repo, tree_sha);
    if (obj->get_type() != "tree") {
        throw std::runtime_error("Object is not a tree: " + tree_sha);
    }
    auto tree = std::dynamic_pointer_cast<GitTree>(obj);
    auto entries = tree->get_entries();
    for (const auto& entry : entries) {
        auto entry_obj = read_object(repo, entry.sha);
        std::string full_path = prefix.empty() ? entry.path : prefix + "/" + entry.path;
        std::cout << entry.mode << " " << entry_obj->get_type() << " " << entry.sha << "\t" << full_path << std::endl;
        if (entry.mode == "40000") {
            tree->recursive_ls_tree(repo, entry.sha, full_path);
        }
    }
}

std::vector<GitTreeEntry> GitTree::get_entries() const {
    return this->entries;
}

std::string branch_sha(const GitRepository &repo, const std::string &branch) {
    fs::path head_path = repo.get_gitdir() / "refs" / "heads" / branch;
    if (!fs::exists(head_path)) {
        throw std::runtime_error("Branch not found: " + branch);
    }
    std::ifstream head_file(head_path);
    std::string sha;
    std::getline(head_file, sha);
    return sha;
}

void tree_checkout(const GitRepository &repo, const std::string &tree_sha, const fs::path &target_path) {
    auto obj = read_object(repo, tree_sha);
    if (!obj) {
        throw std::runtime_error("Object not found.");
    }
    if (obj->get_type() != "tree") {
        throw std::runtime_error("Object is not a tree: " + tree_sha);
    }
    auto tree = std::dynamic_pointer_cast<GitTree>(obj);
    auto entries = tree->get_entries();
    for (const auto &entry : entries) {
        auto entry_obj = read_object(repo, entry.sha);
        if (!entry_obj) {
            throw std::runtime_error("Object not found.");
        }
        fs::path entry_path = target_path / entry.path;
        if (entry_obj->get_type() == "tree") {
            fs::create_directories(entry_path);
            tree_checkout(repo, entry.sha, entry_path);
        }
        else if (entry_obj->get_type() == "blob") {
            std::shared_ptr<GitBlob> blob_obj = std::dynamic_pointer_cast<GitBlob>(entry_obj);
            std::ofstream ofs(entry_path, std::ios::binary);
            ofs << blob_obj->serialize();
        }
        else {
            throw std::runtime_error("Unsupported object type: " + entry_obj->get_type());
        }
    }
}