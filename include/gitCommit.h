#ifndef GIT_COMMIT_H
#define GIT_COMMIT_H

#include <algorithm>
#include <string>
#include <filesystem>

#include "repository.h"
#include "object.h"

namespace fs = std::filesystem;

struct KVLMEntry {
    std::string key;
    std::string value;
};
    
class GitCommit : public GitObject {
public:
    GitCommit(const GitRepository& repo, const std::string& data = "") : GitObject(repo, data) {
        this->fmt = "commit";
    }
    virtual std::string serialize() const override;
    virtual void deserialize(const std::string& data) override;
    std::vector<std::string> get_value(const std::string& key) const;
    std::string get_message() const;
protected:
    std::string message;
    std::vector<KVLMEntry> kvlm;
private:
    std::vector<KVLMEntry> kvlm_parse(const std::string& input);
    std::string serialize_kvlm(const std::vector<KVLMEntry>& kvlm) const;
};

#endif // GIT_COMMIT_H