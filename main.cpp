#include <fstream>
#include <iostream>
#include <iomanip>
#include <string>
#include <set>
#include <vector>
#include <exception>
#include <memory>

#include "repository.h"
#include "object.h"

namespace fs = std::filesystem;

int cmd_init(const std::vector<std::string> &args) {
    if (args.size() < 2) {
        std::cerr << "Usage: init <repository-path>" << std::endl;
        return 1;
    }
    fs::path repo_path = args[2];
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