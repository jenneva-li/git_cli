#include <iostream>

#include "object.h"
#include "gitBlob.h"

GitBlob::GitBlob(const GitRepository& repo, const std::string& data) : GitObject(repo, data) {
    this->fmt = "blob";
    if (!data.empty()) {
        deserialize(data);
    }
}

std::string GitBlob::serialize() const {
    return content;
}

void GitBlob::deserialize(const std::string& data) {
    auto pos = data.find('\0');
    if (pos != std::string::npos) {
        throw std::runtime_error("Invalid blob data: contains null byte");
    }
    this->content = data.substr(pos + 1);
    this->size = content.size();
}
