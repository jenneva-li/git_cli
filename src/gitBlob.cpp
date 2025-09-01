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
    this->content = data;
    this->size = data.size();
}
