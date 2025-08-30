#include <iostream>

#include "object.h"

class GitBlob : public GitObject {
public:
    GitBlob(const GitRepository& repo, const std::string& data = "");
    virtual std::string serialize() const override;
    virtual void deserialize(const std::string& data) override;
};
