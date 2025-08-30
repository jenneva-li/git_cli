#include <iostream>
#include <fstream>
#include <filesystem>
#include <stdexcept>
#include <vector>

#include "repository.h"
#include "configParser.h"

namespace fs = std::filesystem;

ConfigParser GitRepository::config;

GitRepository::GitRepository(const fs::path& path, bool force) 
{
    worktree = path;
    gitdir = path / ".git";

    if (!force && !fs::is_directory(gitdir)) 
    {
        throw std::runtime_error("Not a Git repository: " + path.string());
    }
    configFile = repo_file(*this, "config");

    if(fs::exists(configFile)){
        config.load(configFile);
    }
    else if(!force)
    {
        throw std::runtime_error("Configuration file missing");
    }
    if (!force)
    {
        std::string version = config.get("core", "repositoryformatversion");
        if (version != "0") {
            throw std::runtime_error("Unsupported repository format version: " + version);
        }
    }
}


fs::path GitRepository::repo_dir(const GitRepository& repo, const fs::path& subpath, bool mkdir) {
    fs::path dir = repo.gitdir / subpath;

    if (fs::exists(dir)) {
        if (!fs::is_directory(dir)) {
            throw std::runtime_error("Not a directory: " + dir.string());
        }
    } else if (mkdir) {
        fs::create_directories(dir);
    } else {
        throw std::runtime_error("Directory does not exist: " + dir.string());
    }

    return dir;
}

fs::path GitRepository::repo_file(const GitRepository& repo, const fs::path& file, bool mkdir) {
    fs::path parent = file.parent_path();
    if (!parent.empty()) {
        repo_dir(repo, parent, mkdir);
    }
    return repo.gitdir / file;
}

GitRepository GitRepository::repo_create(const fs::path& path) 
{
    auto repo = GitRepository(path, true);
    if (fs::exists(repo.worktree)) 
    {
        if (!fs::is_directory(repo.worktree))
            throw std::runtime_error("Not a directory: " + path.string());
        if (!fs::is_empty(repo.worktree))
            throw std::runtime_error("Directory not empty: " + path.string());
    } 
    else 
    {
        fs::create_directories(path);
    }

    std::vector<std::string> dirs = {
        "branches", "objects", "refs/tags", "refs/heads"
    };
    for (size_t i = 0; i < dirs.size(); i++) {
        repo_dir(repo, dirs[i], true);
    }
    // Write description
    {
        std::ofstream fd(repo_file(repo, "description"));
        fd << "Unnamed repository; edit this file 'description' to name the repository." << std::endl;
    }
    // Write HEAD
    {
        std::ofstream fh(repo_file(repo, "HEAD"));
        fh << "ref: refs/heads/master" << std::endl;
    }
    // Write config
    {
        std::ofstream fc(repo_file(repo, "config"));
        std::string defaultConfig = repo.config.repo_default_config();
        fc << defaultConfig<< std::endl;
    }
    return repo;
}

GitRepository GitRepository::repo_find(const fs::path& path, bool required) 
{
    fs::path gitdir = path / ".git";
    if (fs::is_directory(gitdir)) {
        return GitRepository(path);
    }
    fs::path parent = path.parent_path();
    if(path==parent)
    {
        if(required)
        {
            throw std::runtime_error("No Git repository found");
        }
        else
        {
            return GitRepository(fs::path());
        }
    }
    return repo_find(parent, required);
}