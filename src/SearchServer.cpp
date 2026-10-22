#include "SearchServer.h"

#include <algorithm>
#include <cctype>
#include <map>
#include <set>
#include <sstream>
#include <utility>

namespace {

std::string normalizeWord(const std::string& source) {
    std::string word;
    word.reserve(source.size());
    for (const unsigned char character : source) {
        if (std::isalpha(character) != 0) {
            word.push_back(static_cast<char>(std::tolower(character)));
        }
    }
    return word;
}

std::vector<std::string> getUniqueWords(const std::string& query) {
    std::set<std::string> unique_words;
    std::istringstream input(query);
    std::string token;
    while (input >> token) {
        const std::string word = normalizeWord(token);
        if (!word.empty()) {
            unique_words.insert(word);
        }
    }
    return {unique_words.begin(), unique_words.end()};
}

}  // namespace

SearchServer::SearchServer(InvertedIndex& index, std::size_t max_responses)
    : index_(index), max_responses_(max_responses == 0 ? 5 : max_responses) {}

std::vector<std::vector<RelativeIndex>> SearchServer::search(
    const std::vector<std::string>& queries_input) {
    std::vector<std::vector<RelativeIndex>> all_results;
    all_results.reserve(queries_input.size());
    total_results_.clear();
    total_results_.reserve(queries_input.size());

    for (const std::string& query : queries_input) {
        std::vector<std::string> words = getUniqueWords(query);
        std::sort(words.begin(), words.end(), [this](const std::string& left,
                                                     const std::string& right) {
            const auto left_entries = index_.GetWordCount(left);
            const auto right_entries = index_.GetWordCount(right);
            const auto total = [](const std::vector<Entry>& entries) {
                std::size_t result = 0;
                for (const Entry& entry : entries) {
                    result += entry.count;
                }
                return result;
            };
            const std::size_t left_total = total(left_entries);
            const std::size_t right_total = total(right_entries);
            return left_total == right_total ? left < right
                                             : left_total < right_total;
        });

        std::map<std::size_t, std::size_t> absolute_relevance;
        for (const std::string& word : words) {
            for (const Entry& entry : index_.GetWordCount(word)) {
                absolute_relevance[entry.doc_id] += entry.count;
            }
        }

        total_results_.push_back(absolute_relevance.size());
        if (absolute_relevance.empty()) {
            all_results.emplace_back();
            continue;
        }

        std::size_t maximum = 0;
        for (const auto& [doc_id, value] : absolute_relevance) {
            static_cast<void>(doc_id);
            maximum = std::max(maximum, value);
        }

        std::vector<RelativeIndex> result;
        result.reserve(absolute_relevance.size());
        for (const auto& [doc_id, value] : absolute_relevance) {
            result.push_back({doc_id, static_cast<float>(value) /
                                          static_cast<float>(maximum)});
        }

        std::sort(result.begin(), result.end(),
                  [](const RelativeIndex& left, const RelativeIndex& right) {
                      return left.rank == right.rank
                                 ? left.doc_id < right.doc_id
                                 : left.rank > right.rank;
                  });

        if (result.size() > max_responses_) {
            result.resize(max_responses_);
        }
        all_results.push_back(std::move(result));
    }

    return all_results;
}

const std::vector<std::size_t>& SearchServer::GetTotalResults() const {
    return total_results_;
}

