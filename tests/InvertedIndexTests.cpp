#include "InvertedIndex.h"

#include <gtest/gtest.h>

#include <string>
#include <vector>

namespace {

void checkIndex(const std::vector<std::string>& documents,
                const std::vector<std::string>& requests,
                const std::vector<std::vector<Entry>>& expected) {
    InvertedIndex index;
    index.UpdateDocumentBase(documents);

    std::vector<std::vector<Entry>> result;
    for (const std::string& request : requests) {
        result.push_back(index.GetWordCount(request));
    }
    EXPECT_EQ(result, expected);
}

}  // namespace

TEST(InvertedIndexTests, CountsWordsInEveryDocument) {
    checkIndex(
        {"london is the capital of great britain",
         "big ben is the nickname for the great bell of the striking clock"},
        {"london", "the"},
        {{{0, 1}}, {{0, 1}, {1, 3}}});
}

TEST(InvertedIndexTests, CountsRepeatedWords) {
    checkIndex(
        {"milk milk milk milk water water",
         "milk water water",
         "milk milk milk milk milk water water water water water",
         "americano cappuccino"},
        {"milk", "water", "cappuccino"},
        {{{0, 4}, {1, 1}, {2, 5}},
         {{0, 2}, {1, 2}, {2, 5}},
         {{3, 1}}});
}

TEST(InvertedIndexTests, ReturnsEmptyListForMissingWord) {
    checkIndex({"abcdefghijkl", "statement"},
               {"m", "statement"},
               {{}, {{1, 1}}});
}

TEST(InvertedIndexTests, RebuildReplacesPreviousIndex) {
    InvertedIndex index;
    index.UpdateDocumentBase({"old old"});
    index.UpdateDocumentBase({"new"});
    EXPECT_TRUE(index.GetWordCount("old").empty());
    EXPECT_EQ(index.GetWordCount("new"), std::vector<Entry>({{0, 1}}));
}

