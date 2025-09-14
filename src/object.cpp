#include <iostream>
#include <filesystem>
#include <stdexcept>
#include <iomanip>
#include <zlib.h>
#include <vector>
#include <algorithm>

#include "repository.h"
#include "sha1/sha1.hpp"
#include "object.h"
#include "gitBlob.h"
#include "gitCommit.h"
#include "gitTree.h"

namespace fs = std::filesystem;

GitObject::GitObject(const GitRepository& repo, const std::string& data) {
    this->repo = &repo; 
    this->content = data;
    this->size = data.size();
}

std::string GitObject::serialize() const {
    throw std::runtime_error("Not implemented");
}

void GitObject::deserialize(const std::string& data) {
    throw std::runtime_error("Not implemented");
}

std::string GitObject::get_type() const {
    return fmt;
}

size_t GitObject::get_size() const {
    return size;
}

std::string GitObject::get_content() const {
    return content;
}

std::vector<unsigned char> compress_data(const std::vector<unsigned char>& data) {
    uLongf compressed_size = compressBound(data.size());
    std::vector<unsigned char> compressed_data(compressed_size);
    if (compress(compressed_data.data(), &compressed_size, data.data(), data.size()) != Z_OK) {
        throw std::runtime_error("Failed to compress data");
    }
    compressed_data.resize(compressed_size);
    return compressed_data;
}

std::vector<unsigned char> decompress_data(const std::vector<unsigned char>& compressed_data) {
    uLongf buffer = 1000000000;
    std::vector<unsigned char> decompressed_data(buffer);
    if (uncompress(decompressed_data.data(), &buffer, compressed_data.data(), compressed_data.size()) != Z_OK) {
        throw std::runtime_error("Failed to decompress data");
    }
    decompressed_data.resize(buffer);
    return decompressed_data;
}

std::shared_ptr<GitObject> read_object(const GitRepository &repo, const std::string &sha) {
    fs::path path = GitRepository::repo_file(repo, "objects/" + sha.substr(0, 2) + "/" + sha.substr(2));
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Failed to open object file");
    }
    
    std::vector<unsigned char> compressed_data;
    char dataByte;
    while (file.get(dataByte)) {
        compressed_data.push_back(static_cast<unsigned char>(dataByte));
    }
    file.close();

    if (compressed_data.empty()) {
        throw std::runtime_error("Empty object file");
    }

    std::vector<unsigned char> decompressed_data = decompress_data(compressed_data);
    auto space_pos = std::find(decompressed_data.begin(), decompressed_data.end(), static_cast<unsigned char>(' '));
    auto null_pos = std::find(decompressed_data.begin(), decompressed_data.end(), static_cast<unsigned char>('\0'));
    if (space_pos == decompressed_data.end() || null_pos == decompressed_data.end()) {
        throw std::runtime_error("Invalid object format: space or null not found");
    }
    std::string original_size_str(reinterpret_cast<char*>(&*(space_pos+1)), std::distance(space_pos+1, null_pos));
    std::string fmt(decompressed_data.begin(), space_pos);
    std::vector<unsigned char> content(null_pos + 1, decompressed_data.end());
    std::shared_ptr<GitObject> obj;
    if (fmt == "blob") {
        obj = std::make_shared<GitBlob>(repo);
    }
    else if (fmt == "commit") {
        obj = std::make_shared<GitCommit>(repo);
    }
    else if (fmt == "tree") {
        obj = std::make_shared<GitTree>(repo);
    }
    else {
        throw std::runtime_error("Unknown object type: " + fmt);
    }
    obj->deserialize(std::string(content.begin(), content.end()));
    return obj;
};

std::string write_object(const GitRepository &repo, const GitObject &obj) {
    std::string serialize_data = obj.serialize();
    std::vector<unsigned char> data(serialize_data.begin(), serialize_data.end());
    std::string header_str = obj.get_type() + " " + std::to_string(data.size()) + std::string(1, '\0') + std::string(data.begin(), data.end());
    
    SHA1 hasher;
    hasher.update(header_str);
    std::string sha = hasher.final();
    std::vector<unsigned char> header(header_str.begin(), header_str.end());
    std::vector<unsigned char> compressed_data = compress_data(header);
    fs::path file = fs::path("objects") / sha.substr(0, 2) / sha.substr(2);
    fs::path path = GitRepository::repo_file(repo, file, true);
    if (!fs::exists(path)) {
        std::ofstream file(path, std::ios::binary);
        file.write(reinterpret_cast<const char*>(compressed_data.data()), compressed_data.size());
    }
    return sha;
}

std::string find_object(const GitRepository &repo, const std::string &sha) {
    if (sha.size() == 40) {
        return sha;
    }
    else {
        std::vector<std::string> matches;
        fs::path dir = repo.get_gitdir() / "objects" / sha.substr(0, 2);
        std::string rest = sha.substr(2);
        if (fs::exists(dir)) {
           for (const auto &entry : fs::directory_iterator(dir)) {
               std::string filename = entry.path().filename().string();
                if (filename.compare(0, rest.size(), rest) == 0) {
                   matches.push_back(sha.substr(0, 2) + filename);
                }
            }
            if (matches.empty()) {
                throw std::runtime_error("Object not found");
            }
            if(matches.size() > 1) {
               throw std::runtime_error("Ambiguous object reference");
            }
        }
        else {
            throw std::runtime_error("Object not found");
        }
        return matches.front();
    }
}

std::string hash_object(const GitRepository &repo, const std::string &data, const std::string &fmt, bool write) {
    std::shared_ptr<GitObject> obj;
    if (fmt == "blob" || fmt.empty()) {
        obj = std::make_shared<GitBlob>(repo, data);
    }
    else {
        throw std::runtime_error("Unknown object type: " + fmt);
    }
    std::string sha = write_object(repo, *obj);
    if (write == true) {
        fs::path file = fs::path("objects") / sha.substr(0, 2) / sha.substr(2);
        fs::path path = GitRepository::repo_file(repo, file);
    }
    return sha;
}