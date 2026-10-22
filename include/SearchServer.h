#pragma once

#include "InvertedIndex.h"

#include <cstddef>
#include <string>
#include <vector>

struct RelativeIndex {
    std::size_t doc_id{};
    float rank{};

    bool operator==(const RelativeIndex& other) const {
        return doc_id == other.doc_id && rank == other.rank;
    }
};

class SearchServer {
public:
    explicit SearchServer(InvertedIndex& index, std::size_t max_responses = 5);

    std::vector<std::vector<RelativeIndex>> search(
        const std::vector<std::string>& queries_input);

    const std::vector<std::size_t>& GetTotalResults() const;

private:
    InvertedIndex& index_;
    std::size_t max_responses_;
    std::vector<std::size_t> total_results_;
};

