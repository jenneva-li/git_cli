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

namespace fs = std::filesystem;

GitObject::GitObject(const GitRepository& repo, const std::string& data) 
{
    this->repo = &repo; 
    if (!data.empty()) 
    {
        deserialize(data);
    }
}

std::string GitObject::serialize() const 
{
    throw std::runtime_error("Not implemented");
}

void GitObject::deserialize(const std::string& data) 
{
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

std::vector<unsigned char> compress_data(const std::vector<unsigned char>& data)
{
    uLongf compressed_size = compressBound(data.size());
    std::vector<unsigned char> compressed_data(compressed_size);
    if (compress(compressed_data.data(), &compressed_size, data.data(), data.size()) != Z_OK)
    {
        throw std::runtime_error("Failed to compress data");
    }
    compressed_data.resize(compressed_size);
    return compressed_data;
}

std::vector<unsigned char> decompress_data(const std::vector<unsigned char>& compressed_data, uLong original_size)
{
    uLongf decompressed_size = original_size;
    std::vector<unsigned char> decompressed_data(decompressed_size);
    if (uncompress(decompressed_data.data(), &decompressed_size, compressed_data.data(), compressed_data.size()) != Z_OK)
    {
        throw std::runtime_error("Failed to decompress data");
    }
    decompressed_data.resize(decompressed_size);
    return decompressed_data;
}

std::shared_ptr<GitObject> read_object(const GitRepository &repo, const std::string &sha)
{
    fs::path path = GitRepository::repo_file(repo, "objects/" + sha.substr(0, 2) + "/" + sha.substr(2));

    std::ifstream file(path, std::ios::binary);
    if (!file)
    {
        throw std::runtime_error("Failed to open object file");
    }
    std::vector<unsigned char> compressed_data;
    char dataByte;
    while (file.get(dataByte))
    {
        compressed_data.push_back(static_cast<unsigned char>(dataByte));
    }
    file.close();

    if (compressed_data.empty())
    {
        throw std::runtime_error("Empty object file");
    }
    auto space_pos = std::find(compressed_data.begin(), compressed_data.end(), static_cast<unsigned char>(' '));
    auto null_pos = std::find(compressed_data.begin(), compressed_data.end(), static_cast<unsigned char>('\0'));
    if (space_pos == compressed_data.end() || null_pos == compressed_data.end()) {
        throw std::runtime_error("Invalid object format: space or null not found");
    }
    std::string original_size_str(space_pos+1, null_pos);
    uLong original_size = std::stoul(original_size_str);
    std::vector<unsigned char> decompressed_data = decompress_data(compressed_data, original_size);

    std::string fmt(decompressed_data.begin(), space_pos);

    std::shared_ptr<GitObject> obj;
    if (fmt == "blob")
    {
        obj = std::make_shared<GitBlob>(repo);
    }
    // else if (fmt == "commit")
    // {
    //     obj = std::make_shared<GitCommit>(repo);
    // }
    // else if (fmt == "tree")
    // {
    //     obj = std::make_shared<GitTree>(repo);
    // }
    else
    {
        throw std::runtime_error("Unknown object type: " + fmt);
    }
    obj->deserialize(std::string(decompressed_data.begin(), decompressed_data.end()));
    return obj;
};

std::string write_object(const GitRepository &repo, const GitObject &obj)
{
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
   if (!fs::exists(path)) 
    {
        std::ofstream file(path, std::ios::binary);
        file.write(reinterpret_cast<const char*>(compressed_data.data()), compressed_data.size());
    }
   return sha;
}