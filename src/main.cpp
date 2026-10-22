#include "ConverterJSON.h"
#include "InvertedIndex.h"
#include "SearchServer.h"

#include <exception>
#include <iostream>
#include <utility>
#include <vector>

int main() {
    try {
        ConverterJSON converter;
        std::cout << "Starting " << converter.GetName() << " v"
                  << converter.GetVersion() << '\n';

        InvertedIndex index;
        const std::vector<std::string> documents = converter.GetTextDocuments();
        index.UpdateDocumentBase(documents);

        const std::vector<std::string> requests = converter.GetRequests();
        SearchServer server(
            index, static_cast<std::size_t>(converter.GetResponsesLimit()));
        const auto search_results = server.search(requests);

        std::vector<std::vector<std::pair<int, float>>> answers;
        answers.reserve(search_results.size());
        for (const auto& request_results : search_results) {
            std::vector<std::pair<int, float>> answer;
            answer.reserve(request_results.size());
            for (const RelativeIndex& result : request_results) {
                answer.emplace_back(static_cast<int>(result.doc_id), result.rank);
            }
            answers.push_back(std::move(answer));
        }

        converter.putAnswers(answers);
        for (std::size_t index = 0; index < server.GetTotalResults().size();
             ++index) {
            std::cout << "request" << index + 1 << ": found "
                      << server.GetTotalResults()[index] << " document(s)\n";
        }
        std::cout << "Search completed. Results saved to answers.json\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}

