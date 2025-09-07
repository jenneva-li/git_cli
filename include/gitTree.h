#ifndef GIT_TREE_H
#define GIT_TREE_H

#include <string>
#include <algorithm>
#include <sstream>
#include <vector>   
#include <array>

#include "object.h"

struct GitTreeEntry {
    std::string mode;
    std::string path;
    std::string sha;
};

class GitTree : public GitObject {
public:
    GitTree(const GitRepository& repo, const std::string& data = "") : GitObject(repo, data) {
        this->fmt = "tree";
    }
    virtual std::string serialize() const override;
    virtual void deserialize(const std::string& data) override;
    std::vector<GitTreeEntry> parse_tree(const std::vector<unsigned char>& data);
    void recursive_ls_tree(const GitRepository& repo, const std::string& tree_sha, const std::string& prefix="");
    std::vector<GitTreeEntry> get_entries() const;
protected:
    std::vector<GitTreeEntry> entries;
private:
    std::pair<size_t, GitTreeEntry> parse_single_tree(const std::vector<unsigned char>& data, size_t pos);
    std::vector<GitTreeEntry> sort_tree_leaf(const std::vector<GitTreeEntry>& entries) const;
    std::string serialize_tree(const std::vector<GitTreeEntry>& entries) const;
};

#endif // GIT_TREE_H
