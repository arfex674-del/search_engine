#include "SearchServer.h"

#include <gtest/gtest.h>

#include <string>
#include <vector>

TEST(SearchServerTests, CalculatesRelativeRelevance) {
    const std::vector<std::string> documents = {
        "milk milk milk milk water water water",
        "milk water water",
        "milk milk milk milk milk water water water water water",
        "americano cappuccino",
    };
    const std::vector<std::string> requests = {"milk water", "sugar"};
    const std::vector<std::vector<RelativeIndex>> expected = {
        {{2, 1.0F}, {0, 0.7F}, {1, 0.3F}},
        {},
    };

    InvertedIndex index;
    index.UpdateDocumentBase(documents);
    SearchServer server(index);

    EXPECT_EQ(server.search(requests), expected);
    EXPECT_EQ(server.GetTotalResults(), std::vector<std::size_t>({3, 0}));
}

TEST(SearchServerTests, ReturnsOnlyTopFiveByDefault) {
    const std::vector<std::string> documents = {
        "london is the capital of great britain",
        "paris is the capital of france",
        "berlin is the capital of germany",
        "rome is the capital of italy",
        "madrid is the capital of spain",
        "lisboa is the capital of portugal",
        "bern is the capital of switzerland",
        "moscow is the capital of russia",
        "kiev is the capital of ukraine",
        "minsk is the capital of belarus",
        "astana is the capital of kazakhstan",
        "beijing is the capital of china",
        "tokyo is the capital of japan",
        "bangkok is the capital of thailand",
        "welcome to moscow the capital of russia the third rome",
        "amsterdam is the capital of netherlands",
        "helsinki is the capital of finland",
        "oslo is the capital of norway",
        "stockholm is the capital of sweden",
        "riga is the capital of latvia",
        "tallinn is the capital of estonia",
        "warsaw is the capital of poland",
    };
    const std::vector<std::vector<RelativeIndex>> expected = {{
        {7, 1.0F},
        {14, 1.0F},
        {0, 0.666666687F},
        {1, 0.666666687F},
        {2, 0.666666687F},
    }};

    InvertedIndex index;
    index.UpdateDocumentBase(documents);
    SearchServer server(index);

    EXPECT_EQ(server.search({"moscow is the capital of russia"}), expected);
    EXPECT_EQ(server.GetTotalResults(), std::vector<std::size_t>({22}));
}

TEST(SearchServerTests, DoesNotCountRepeatedQueryWordsTwice) {
    InvertedIndex index;
    index.UpdateDocumentBase({"cat cat", "cat dog"});
    SearchServer server(index, 10);

    EXPECT_EQ(server.search({"cat cat"}),
              std::vector<std::vector<RelativeIndex>>(
                  {{{0, 1.0F}, {1, 0.5F}}}));
}

