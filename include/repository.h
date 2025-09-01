#ifndef REPOSITORY_H
#define REPOSITORY_H

#include <string>
#include <filesystem>

#include "configParser.h"

namespace fs = std::filesystem;

class GitRepository
{
public:
    GitRepository(const fs::path &path, bool force = false);
    static ConfigParser config;
    static GitRepository repo_create(const fs::path &path);
    static fs::path repo_file(const GitRepository &repo, const fs::path &file, bool mkdir = false);
    static GitRepository repo_find(const fs::path &path=".", bool required = true);
    fs::path get_gitdir() const {
        return gitdir;
    }
protected:
    fs::path worktree;
    fs::path gitdir;
    fs::path configFile;
    static fs::path repo_dir(const GitRepository &repo, const fs::path &dir, bool mkdir = false);
};

#endif // REPOSITORY_H