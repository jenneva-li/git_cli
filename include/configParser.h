#ifndef CONFIGPARSER_H
#define CONFIGPARSER_H

#include <unordered_map>
#include <string>

class ConfigParser
{
public:
    void load(const std::filesystem::path &configFile);
    std::string get(const std::string &section, const std::string &key) const;
    std::string repo_default_config();
    std::string get_configData() const;

private:
    std::unordered_map<std::string, std::unordered_map<std::string, std::string>> configData;
};

#endif // CONFIGPARSER_H