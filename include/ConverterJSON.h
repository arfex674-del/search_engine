#pragma once

#include <filesystem>
#include <string>
#include <utility>
#include <vector>

class ConverterJSON {
public:
    explicit ConverterJSON(
        std::filesystem::path config_path = "config.json",
        std::filesystem::path requests_path = "requests.json",
        std::filesystem::path answers_path = "answers.json");

    std::vector<std::string> GetTextDocuments() const;
    int GetResponsesLimit() const;
    std::vector<std::string> GetRequests() const;
    void putAnswers(
        const std::vector<std::vector<std::pair<int, float>>>& answers) const;

    std::string GetName() const;
    std::string GetVersion() const;

private:
    std::filesystem::path config_path_;
    std::filesystem::path requests_path_;
    std::filesystem::path answers_path_;
};

