#include <iostream>
#include <fstream>
#include <string>
#include <unordered_map>
#include <stdexcept>
#include <sstream>
#include <filesystem>

#include "configParser.h"

void ConfigParser::load(const std::filesystem::path &configFile)
{
    std::ifstream cf(configFile);
    if (!cf)
    {
        throw std::runtime_error("Error opening config file.");
    }
    std::string line, section;
    while (std::getline(cf, line))
    {
        if (line.empty() || line[0] == '#' || line[0] == ';')
            continue; 

        if (line.front() == '[' && line.back() == ']')
        {
            section = line.substr(1, line.size() - 2);
        }
        
        if (line.find('=') != std::string::npos)
        {
            std::string key = line.substr(0, line.find('='));
            std::string value = line.substr(line.find('=') + 1);
            configData[section][key] = value;
        }
    }
}
std::string ConfigParser::get(const std::string &section, const std::string &key) const
{
    auto outer = configData.find(section);
    if (outer != configData.end())
    {
        auto inner = outer->second.find(key);
        if (inner != outer->second.end())
        {
            return inner->second;
        }
    }
    throw std::runtime_error("Key not found");
    return "";
}
std::string ConfigParser::repo_default_config()
{
    configData["core"];
    configData["core"]["repositoryformatversion"] = "0";
    configData["core"]["filemode"] = "false";
    configData["core"]["bare"] = "false";
    return get_configData();
}

std::string ConfigParser::get_configData() const
{
    std::ostringstream oss;
    for (const auto& section : configData)
    {
        oss << "[" << section.first << "]\n";
        for (const auto& [key, value] : section.second)
        {
            oss << key << "=" << value << "\n";
        }
    }
    return oss.str();
}