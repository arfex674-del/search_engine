#include "InvertedIndex.h"

#include <algorithm>
#include <cctype>
#include <future>
#include <sstream>
#include <unordered_map>

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

std::unordered_map<std::string, std::size_t> countWords(
    const std::string& document) {
    std::unordered_map<std::string, std::size_t> frequencies;
    std::istringstream input(document);
    std::string token;
    while (input >> token) {
        const std::string word = normalizeWord(token);
        if (!word.empty()) {
            ++frequencies[word];
        }
    }
    return frequencies;
}

}  // namespace

void InvertedIndex::UpdateDocumentBase(
    const std::vector<std::string>& input_docs) {
    docs_ = input_docs;
    freq_dictionary_.clear();

    std::vector<std::future<std::unordered_map<std::string, std::size_t>>> jobs;
    jobs.reserve(docs_.size());
    for (const std::string& document : docs_) {
        jobs.push_back(std::async(std::launch::async, countWords, document));
    }

    for (std::size_t doc_id = 0; doc_id < jobs.size(); ++doc_id) {
        const auto frequencies = jobs[doc_id].get();
        for (const auto& [word, count] : frequencies) {
            freq_dictionary_[word].push_back({doc_id, count});
        }
    }
}

std::vector<Entry> InvertedIndex::GetWordCount(const std::string& word) const {
    const std::string normalized = normalizeWord(word);
    const auto iterator = freq_dictionary_.find(normalized);
    if (iterator == freq_dictionary_.end()) {
        return {};
    }
    return iterator->second;
}

