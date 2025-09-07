#include <fstream>
#include <iostream>
#include <iomanip>
#include <string>
#include <set>
#include <vector>
#include <exception>
#include <memory>
#include <map>

#include "repository.h"
#include "object.h"
#include "gitCommit.h"
#include "gitTree.h"

namespace fs = std::filesystem;

int cmd_init(const std::vector<std::string> &args) {
    if (args.size() < 1) {
        std::cerr << "Usage: init <repository-path>" << std::endl;
        return 1;
    }
    fs::path repo_path = fs::current_path();
    GitRepository::repo_create(repo_path);
    return 0;
}

int cmd_cat_file(const std::vector<std::string> &args) {
    if (args.size() < 4) {
        std::cerr << "Usage: cat-file <type> <object>" << std::endl;
        return 1;
    }
    const std::string &type = args[2];
    const std::string &object = args[3];
    
    try {
        GitRepository repo = GitRepository::repo_find(fs::current_path(), true);
        std::string obj_name = find_object(repo, object);
        std::shared_ptr<GitObject> obj = read_object(repo, obj_name);
        if (type == "-p") {
            std::cout << obj->get_content() << std::endl;
            return 0;
        } else if (type == "-t") {
            std::cout << obj->get_type() << std::endl;
            return 0;
        } else if (type == "-s") {
            std::cout << obj->get_size() << std::endl;
            return 0;
        } else {
            std::cerr << "Unknown option: " << type <<std::endl;
            std::cerr << "Options: -p, -t, -s" << std::endl;
            return 1;
        }
    } 
    catch (const std::exception &e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}

int cmd_hash_object(const std::vector<std::string> &args) {
    if (args.size() < 4) {
        std::cerr << "Usage: hash-object [-t <type>] [-w] [--path=<file>]" << std::endl;
        return 1;
    }
    const std::string &type = args[2];
    bool write = false;
    fs::path file;

    if (args[3] == "-w") {
        write = true;
        if (args.size() < 5) {
            std::cerr << "Usage: hash-object [-t <type>] [-w] [--path=<file>]" << std::endl;
            return 1;
        }
        file = args[4];
    } else {
        file = args[3];
    }

    try {
        GitRepository repo = GitRepository::repo_find(fs::current_path(), true);
        if (!fs::exists(file) || !fs::is_regular_file(file)) {
            throw std::runtime_error("File not found: " + file.string());
        }
        std::ifstream ifs(file);
        std::string data((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
        std::string sha = hash_object(repo, data, type, write);
        std::cout << sha << std::endl;
        return 0;
    }
    catch (const std::exception &e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}

int log_graphviz(GitRepository &repo, const std::string& sha, std::set<std::string>& seen) {

    if (seen.count(sha)) 
        return 0;
    seen.insert(sha);
    
    const auto& commit_obj = read_object(repo, sha);
    if (commit_obj == nullptr) {
        throw std::runtime_error("Object not found: " + sha);
        return 1;
    }
    else if (commit_obj->get_type() != "commit") {
        throw std::runtime_error("Not a commit object: " + sha);
        return 1;
    }
    auto commit = std::dynamic_pointer_cast<GitCommit>(commit_obj);
    std::string msg = commit->get_message();
    std::cout << " c_" << sha << " [label=\"" << sha.substr(0, 7) << ": " << msg << "\"];\n";

    auto parents = commit->get_value("parent");
    for (const std::string& parent : parents) {
        std::cout << " c_" << sha << " -> c_" << parent << ";\n";
        log_graphviz(repo, parent, seen);
    }
    return 0;
}

int cmd_log(const std::vector<std::string> &args) {
    int status = 0;
    
    if (args.size() < 3) {
        std::cerr << "Usage: log <commit>" << std::endl;
        return 1;
    }
    const std::string &commit = args[2];
    
    try {
        GitRepository repo = GitRepository::repo_find(fs::current_path(), true);
        std::string obj_name = find_object(repo, commit);
        std::shared_ptr<GitObject> obj = read_object(repo, obj_name);
        if (obj->get_type() != "commit") {
            throw std::runtime_error("Object is not a commit: " + commit);
        }
        std::cout<< "digraph log {" << std::endl;
        std::set<std::string> seen;
        status = log_graphviz(repo, obj_name, seen);
        if (status == 0) {
            std::cout << "}\n";
        }
    } 
    catch (const std::exception &e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return status;
}

int cmd_ls_tree(const std::vector<std::string> &args) {
    if (args.size() < 2) {
        std::cerr << "Usage: ls-tree [options] <tree-ish> [path]" << std::endl;
        std::cerr << "Options: -r, --name-only, --long" << std::endl;
        return 1;
    }
    std::string treeish;
    std::string path;
    bool opt_recursive = false;
    bool opt_name_only = false;
    bool opt_long = false;

    for (size_t i = 2; i < args.size(); ++i) {
        const std::string &arg = args[i];
        if (arg == "-r") {
            opt_recursive = true;
        } else if (arg == "--name-only") {
            opt_name_only = true;
        } else if (arg == "--long") {
            opt_long = true;
        } else if (treeish.empty()) {
            treeish = arg;
        } else {
            path = arg;
        }
    }
   
    GitRepository repo = GitRepository::repo_find(fs::current_path(), true);
    std::string tree_sha = find_object(repo, treeish);
    auto obj = read_object(repo, tree_sha);
    if (obj->get_type() != "tree")
    {
        throw std::runtime_error("Object is not a tree: " + tree_sha);
    }
    auto tree = std::dynamic_pointer_cast<GitTree>(obj);
    auto entries = tree->get_entries();
    if (opt_recursive)
    {
        tree->recursive_ls_tree(repo, tree_sha, path);
        return 0;
    }
    for (const auto &entry : entries)
    {
        auto entry_obj = read_object(repo, entry.sha);
        if (opt_name_only)
        {
            std::cout << entry.path << std::endl;
            continue;
        }
        if (opt_long)
        {
            std::cout << entry.mode << " " << entry_obj->get_type() << " " << entry.sha << "\t" << entry_obj->get_size() << "\t" << entry.path << std::endl;
            continue;
        }
        std::cout << entry.mode << " " << entry_obj->get_type() << " " << entry.sha << "\t" << entry.path << std::endl;
    }
    return 0;
}

int process_command(const std::vector<std::string> &args) {
    int status = 0;
    if (args.empty()) {
        std::cerr << "No command provided." << std::endl;
        return 1;
    }

    const std::string& command = args[1];
    if (command == "init") 
        status = cmd_init(args);
    else if (command == "cat-file") 
        status = cmd_cat_file(args);
    else if (command == "hash-object")
        status = cmd_hash_object(args);
    else if (command == "log")
        status = cmd_log(args);
    else if (command == "ls-tree")
        status = cmd_ls_tree(args);
    else {
        std::cerr << "Unknown command: " << command << std::endl;
        status = 1;
    }
    return status;
}

int main(int argc, char *argv[]) {
    std::vector<std::string> args(argv, argv + argc);
    return process_command(args);
}