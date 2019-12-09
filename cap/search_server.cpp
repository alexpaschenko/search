#include "search_server.h"
#include "iterator_range.h"

#include <algorithm>
#include <iterator>
#include <sstream>
#include <iostream>
#include <cstring>

vector<string> SplitIntoWords(const string& line) {
  istringstream words_input(line);
  return {istream_iterator<string>(words_input), istream_iterator<string>()};
}

SearchServer::SearchServer(istream& document_input) {
  UpdateDocumentBase(document_input);
}

void SearchServer::UpdateDocumentBase(istream& document_input) {
  InvertedIndex new_index;

  for (string current_document; getline(document_input, current_document); ) {
    new_index.Add(current_document);
  }

  index = move(new_index);
}

struct DocResult {
    std::size_t first;

    int64_t second;
};

//typedef pair<size_t, int64_t> DOC_RESULT;

typedef DocResult DOC_RESULT;

bool operator>(const DocResult& a, const DocResult& b) {
    if (a.first > b.first) {
        return true;
    } else if (a.first == b.first) {
        return a.second > b.second;
    } else {
        return false;
    }
}

void SearchServer::AddQueriesStream(
  istream& query_input, ostream& search_results_output
) {

    const size_t total_docs = index.count();

    //vector<size_t> docid_count(total_docs);

    vector<DOC_RESULT> search_results(total_docs);

    vector<DOC_RESULT> empty_results(total_docs);

    vector<size_t> idxs;

    search_results.reserve(total_docs);

    idxs.reserve(total_docs);

    unordered_map<string, vector<DOC_RESULT>> cache;

    size_t len = sizeof(size_t) * total_docs;

    for (string current_query; getline(query_input, current_query); ) {
        const auto& cached_result = cache.find(current_query);

        const bool is_cached = (cached_result != cache.end());

        vector<DOC_RESULT>& current_result = (is_cached ? cached_result->second : search_results);

        if (is_cached) {
            goto print;
        }

        {
            /*if (total_docs < 1000) {
                memset(idxs.data(), 0, len);
            } else */{
                search_results.assign(total_docs, {0, 0});

                //fill(search_results.begin(), search_results.end(), {0, 0});


                //memcpy(search_results.data(), empty_results.data(), sizeof(DOC_RESULT) * total_docs);

                //memset(search_results.data(), 0, sizeof(DocResult) * total_docs);
            }

            size_t max_found_docid = 0;

            istringstream words_input(current_query);

            string word;

            while (words_input >> word) {
                for (const size_t docid : index.Lookup(word)) {
                    /*if (total_docs < 1000) {
                        auto &idx = idxs[docid];

                        if (idx == 0) {
                            search_results.emplace_back(1, -docid);

                            idx = search_results.size();
                        } else {
                            search_results[idx - 1].first++;
                        }
                    } else */{
                        auto& doc_pair = search_results[docid];

                        if (!doc_pair.first++) {
                            max_found_docid = max(max_found_docid, docid);

                            doc_pair.second = -docid;
                        }
                    }
                }
            }

            /*set<size_t> counters;

            size_t mn = 0;

            for (auto &p : search_results) {
                if (p.first && (counters.empty() || counters.size() < 5 || p.first > mn)) {
                    counters.insert(p.first);

                    if (counters.size() > 5) {
                        counters.erase(counters.begin());
                    }

                    mn = *counters.begin();
                }
            }*/

            /*for (int64_t i = 0; i < docid_count.size(); i++) {
                if (docid_count[i] > 0) {
                    search_results.emplace_back(docid_count[i], -i);
                }
            }*/

            partial_sort(
                    begin(search_results),
                    begin(search_results) + min((size_t) 5, total_docs),
                    begin(search_results) + max_found_docid + 1, std::greater<>());

            auto resit = Head(current_result, 5);

            vector<DOC_RESULT> found(resit.begin(), resit.end());

            current_result = cache[current_query] = move(found);
        }

    print:
    search_results_output << current_query << ':';
    for (auto [hitcount, docid] : Head(current_result, 5)) {
        if (hitcount == 0) {
            break;
        }

        search_results_output << " {"
        << "docid: " << -docid << ", "
        << "hitcount: " << hitcount << '}';
    }
    search_results_output << '\n';

    if (!is_cached) {
        search_results.clear();
    }
  }
}

void InvertedIndex::Add(const string& document) {
    const size_t docid = last_doc_id++;

    istringstream words_input(document);

    string word;

    while (words_input >> word) {
      //index[word].reserve(8000);

      index[word].push_back(docid);
    }
}

vector<size_t>& InvertedIndex::Lookup(const string& word) {
    return index[word];
}

size_t InvertedIndex::count() const {
    return last_doc_id;
}
