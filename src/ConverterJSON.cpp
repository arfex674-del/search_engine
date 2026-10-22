#include "ConverterJSON.h"

#include <nlohmann/json.hpp>

#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>

namespace {

constexpr const char* kApplicationVersion = "0.1";
constexpr int kDefaultResponsesLimit = 5;

nlohmann::json readJson(const std::filesystem::path& path,
                        const std::string& missing_message) {
    std::ifstream input(path);
    if (!input.is_open()) {
        throw std::runtime_error(missing_message);
    }

    try {
        return nlohmann::json::parse(input);
    } catch (const nlohmann::json::exception& error) {
        throw std::runtime_error(path.string() + " is invalid: " + error.what());
    }
}

nlohmann::json readConfig(const std::filesystem::path& path) {
    nlohmann::json root = readJson(path, "config file is missing");
    if (!root.contains("config") || !root["config"].is_object() ||
        root["config"].empty()) {
        throw std::runtime_error("config file is empty");
    }

    const nlohmann::json& config = root["config"];
    if (!config.contains("version") || !config["version"].is_string() ||
        config["version"].get<std::string>() != kApplicationVersion) {
        throw std::runtime_error("config.json has incorrect file version");
    }

    return root;
}

std::filesystem::path resolveDocumentPath(
    const std::filesystem::path& config_path,
    const std::filesystem::path& document_path) {
    if (document_path.is_absolute()) {
        return document_path;
    }

    const std::filesystem::path parent = config_path.parent_path();
    return parent.empty() ? document_path : parent / document_path;
}

}  // namespace

ConverterJSON::ConverterJSON(std::filesystem::path config_path,
                             std::filesystem::path requests_path,
                             std::filesystem::path answers_path)
    : config_path_(std::move(config_path)),
      requests_path_(std::move(requests_path)),
      answers_path_(std::move(answers_path)) {}

std::vector<std::string> ConverterJSON::GetTextDocuments() const {
    const nlohmann::json root = readConfig(config_path_);
    std::vector<std::string> documents;

    if (!root.contains("files") || !root["files"].is_array()) {
        return documents;
    }

    documents.reserve(root["files"].size());
    for (const auto& value : root["files"]) {
        if (!value.is_string()) {
            std::cerr << "Document path in config.json is not a string\n";
            documents.emplace_back();
            continue;
        }

        const auto path = resolveDocumentPath(
            config_path_, value.get<std::string>());
        std::ifstream input(path);
        if (!input.is_open()) {
            std::cerr << "File is missing: " << path.string() << '\n';
            documents.emplace_back();
            continue;
        }

        std::ostringstream buffer;
        buffer << input.rdbuf();
        documents.push_back(buffer.str());
    }

    return documents;
}

int ConverterJSON::GetResponsesLimit() const {
    const nlohmann::json root = readConfig(config_path_);
    const nlohmann::json& config = root["config"];
    if (!config.contains("max_responses") ||
        !config["max_responses"].is_number_integer()) {
        return kDefaultResponsesLimit;
    }

    const int value = config["max_responses"].get<int>();
    return value > 0 ? value : kDefaultResponsesLimit;
}

std::vector<std::string> ConverterJSON::GetRequests() const {
    const nlohmann::json root = readJson(requests_path_, "requests file is missing");
    if (!root.contains("requests") || !root["requests"].is_array()) {
        throw std::runtime_error("requests file is empty");
    }

    std::vector<std::string> requests;
    requests.reserve(root["requests"].size());
    for (const auto& request : root["requests"]) {
        if (!request.is_string()) {
            throw std::runtime_error("request must be a string");
        }
        requests.push_back(request.get<std::string>());
    }
    return requests;
}

void ConverterJSON::putAnswers(
    const std::vector<std::vector<std::pair<int, float>>>& answers) const {
    nlohmann::json output;
    output["answers"] = nlohmann::json::object();

    const std::size_t limit = static_cast<std::size_t>(GetResponsesLimit());
    for (std::size_t request_index = 0; request_index < answers.size();
         ++request_index) {
        std::ostringstream request_name;
        request_name << "request" << std::setw(3) << std::setfill('0')
                     << request_index + 1;

        nlohmann::json answer;
        if (answers[request_index].empty()) {
            answer["result"] = "false";
        } else {
            answer["result"] = "true";
            answer["relevance"] = nlohmann::json::array();

            const std::size_t count =
                std::min(limit, answers[request_index].size());
            for (std::size_t i = 0; i < count; ++i) {
                answer["relevance"].push_back({
                    {"docid", answers[request_index][i].first},
                    {"rank", answers[request_index][i].second},
                });
            }
        }

        output["answers"][request_name.str()] = std::move(answer);
    }

    std::ofstream file(answers_path_, std::ios::trunc);
    if (!file.is_open()) {
        throw std::runtime_error("cannot create answers file");
    }
    file << std::setw(4) << output << '\n';
}

std::string ConverterJSON::GetName() const {
    const nlohmann::json root = readConfig(config_path_);
    const nlohmann::json& config = root["config"];
    if (!config.contains("name") || !config["name"].is_string()) {
        return "SearchEngine";
    }
    return config["name"].get<std::string>();
}

std::string ConverterJSON::GetVersion() const {
    return readConfig(config_path_)["config"]["version"].get<std::string>();
}

