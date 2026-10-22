#include "ConverterJSON.h"

#include <gtest/gtest.h>
#include <nlohmann/json.hpp>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

namespace {

class TemporaryProject {
public:
    TemporaryProject() {
        const auto suffix = std::chrono::steady_clock::now()
                                .time_since_epoch()
                                .count();
        path = std::filesystem::temp_directory_path() /
               ("search_engine_tests_" + std::to_string(suffix));
        std::filesystem::create_directories(path / "resources");
    }

    ~TemporaryProject() {
        std::error_code error;
        std::filesystem::remove_all(path, error);
    }

    void write(const std::filesystem::path& relative_path,
               const std::string& content) const {
        std::ofstream file(path / relative_path);
        file << content;
    }

    std::filesystem::path path;
};

}  // namespace

TEST(ConverterJSONTests, ReadsConfigurationDocumentsAndRequests) {
    TemporaryProject project;
    project.write("resources/document.txt", "alpha beta");
    project.write(
        "config.json",
        R"({"config":{"name":"Test","version":"0.1","max_responses":3},"files":["resources/document.txt"]})");
    project.write("requests.json", R"({"requests":["alpha","beta"]})");

    ConverterJSON converter(project.path / "config.json",
                            project.path / "requests.json",
                            project.path / "answers.json");

    EXPECT_EQ(converter.GetName(), "Test");
    EXPECT_EQ(converter.GetResponsesLimit(), 3);
    EXPECT_EQ(converter.GetTextDocuments(),
              std::vector<std::string>({"alpha beta"}));
    EXPECT_EQ(converter.GetRequests(),
              std::vector<std::string>({"alpha", "beta"}));
}

TEST(ConverterJSONTests, UsesDefaultResponseLimit) {
    TemporaryProject project;
    project.write(
        "config.json",
        R"({"config":{"name":"Test","version":"0.1"},"files":[]})");
    ConverterJSON converter(project.path / "config.json");
    EXPECT_EQ(converter.GetResponsesLimit(), 5);
}

TEST(ConverterJSONTests, WritesAnswersInRequiredFormat) {
    TemporaryProject project;
    project.write(
        "config.json",
        R"({"config":{"name":"Test","version":"0.1","max_responses":2},"files":[]})");
    ConverterJSON converter(project.path / "config.json",
                            project.path / "requests.json",
                            project.path / "answers.json");

    converter.putAnswers({{{2, 1.0F}, {0, 0.5F}, {1, 0.25F}}, {}});

    std::ifstream input(project.path / "answers.json");
    const nlohmann::json answer = nlohmann::json::parse(input);
    EXPECT_EQ(answer["answers"]["request001"]["result"], "true");
    EXPECT_EQ(answer["answers"]["request001"]["relevance"].size(), 2);
    EXPECT_EQ(answer["answers"]["request002"]["result"], "false");
    EXPECT_FALSE(answer["answers"]["request002"].contains("relevance"));
}

TEST(ConverterJSONTests, ReportsMissingConfiguration) {
    TemporaryProject project;
    ConverterJSON converter(project.path / "config.json");
    try {
        static_cast<void>(converter.GetName());
        FAIL() << "Expected a missing configuration error";
    } catch (const std::runtime_error& error) {
        EXPECT_STREQ(error.what(), "config file is missing");
    }
}

TEST(ConverterJSONTests, ReportsEmptyAndIncorrectConfiguration) {
    TemporaryProject project;
    project.write("config.json", R"({"files":[]})");
    ConverterJSON empty_converter(project.path / "config.json");
    EXPECT_THROW(
        {
            try {
                static_cast<void>(empty_converter.GetName());
            } catch (const std::runtime_error& error) {
                EXPECT_STREQ(error.what(), "config file is empty");
                throw;
            }
        },
        std::runtime_error);

    project.write(
        "config.json",
        R"({"config":{"name":"Test","version":"9.9"},"files":[]})");
    ConverterJSON version_converter(project.path / "config.json");
    EXPECT_THROW(
        {
            try {
                static_cast<void>(version_converter.GetName());
            } catch (const std::runtime_error& error) {
                EXPECT_STREQ(error.what(),
                             "config.json has incorrect file version");
                throw;
            }
        },
        std::runtime_error);
}

TEST(ConverterJSONTests, MissingDocumentDoesNotShiftDocumentIds) {
    TemporaryProject project;
    project.write("resources/second.txt", "second document");
    project.write(
        "config.json",
        R"({"config":{"name":"Test","version":"0.1"},"files":["resources/missing.txt","resources/second.txt"]})");

    ConverterJSON converter(project.path / "config.json");
    const auto documents = converter.GetTextDocuments();
    ASSERT_EQ(documents.size(), 2);
    EXPECT_TRUE(documents[0].empty());
    EXPECT_EQ(documents[1], "second document");
}

