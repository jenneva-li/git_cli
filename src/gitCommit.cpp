#include <algorithm>
#include <string>
#include <vector>

#include "gitCommit.h"

// KVLM: Key-Value List with Message
std::vector<KVLMEntry> GitCommit::kvlm_parse(const std::string &input) {
    std::vector<KVLMEntry> kvlm;
    std::string line;
    std::istringstream stream(input);
    while (std::getline(stream, line)) {
        if (line.empty()) {
            std::string commit_msg;
            std::string msg_line;
            while (std::getline(stream, msg_line)) {
                commit_msg += msg_line + "\n";
            }
            if (!commit_msg.empty() && commit_msg.back() == '\n') {
                commit_msg.pop_back();
            }
            kvlm.push_back({ "commit_msg", commit_msg });
            break;
        }
        else {
            auto space_pos = line.find(" ");
            if (space_pos == std::string::npos) {
                throw std::runtime_error("Invalid KVLM format: no space found");
            }
            std::string key = line.substr(0, space_pos);
            std::string value = line.substr(space_pos + 1);

            while (stream.peek() == ' ') {
                std::string cont_line;
                if (std::getline(stream, cont_line)) {
                    cont_line.erase(0, cont_line.find_first_not_of(" "));
                    value += "\n" + cont_line;
                }
            }
            kvlm.push_back({ key, value });
        }
    }
    return kvlm;
}

std::string GitCommit::serialize_kvlm(const std::vector<KVLMEntry>& kvlm) const {
    std::string header;
    std::string result;
    bool firstline = true;

    for (const auto& entry : kvlm) {
        if (entry.key == "commit_msg") {
            continue;
        }
        std::istringstream valStream(entry.value);
        std::string line;
        if (std::getline(valStream, line)) {
            if (firstline) {
                header += entry.key + " " + line + "\n";
                firstline = false;
            }
            else{
                header += " " + line + "\n";
            }
        }
    }
    result += header + "\n";
    result += this->message;
    return result;
}

std::string GitCommit::serialize() const {
    return serialize_kvlm(kvlm);
}

void GitCommit::deserialize(const std::string& data) {
    this->kvlm = kvlm_parse(data);
    if (!this->kvlm.empty() && this->kvlm.back().key == "commit_msg") {
        this->message = this->kvlm.back().value;
    }
}

std::vector<std::string> GitCommit::get_value(const std::string& key) const {
    std::vector<std::string> values;
    for (const auto& entry : kvlm) {
        if (entry.key == key) {
            values.push_back(entry.value);
        }
    }
    return values;
}

std::string GitCommit::get_message() const {
    return this->message;
}