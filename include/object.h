#ifndef OBJECT_H
#define OBJECT_H

#include <iostream>
#include <filesystem>
#include <stdexcept>
#include <iomanip>
#include <zlib.h>

#include "repository.h"
#include "sha1/sha1.hpp"

namespace fs = std::filesystem;

class GitObject{
public:
    GitObject(const GitRepository& repo, const std::string& data = "");
    virtual std::string serialize() const;
    virtual void deserialize(const std::string& data);
    std::string get_type() const;
    size_t get_size() const;
    std::string get_content() const;
protected:
    const GitRepository* repo;
    std::string fmt;
    size_t size;
    std::string content;
};

std::string find_object(const GitRepository& repo, const std::string& sha);
std::shared_ptr<GitObject> read_object(const GitRepository& repo, const std::string& sha);
std::string write_object(const GitRepository& repo, const std::string& data);

#endif // OBJECT_H