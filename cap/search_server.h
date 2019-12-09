#pragma once

#include <istream>
#include <ostream>
#include <set>
#include <list>
#include <vector>
#include <map>
#include <unordered_map>
#include <string>
using namespace std;

class InvertedIndex {
public:
  void Add(const string& document);

  vector<size_t>& Lookup(const string& word);

  const string& GetDocument(size_t id) const {
    return docs[id];
  }

  size_t count() const;

private:
  unordered_map<string, vector<size_t>> index;

  vector<string> docs;

  size_t last_doc_id = 0;
};

class SearchServer {
public:
  SearchServer() = default;
  explicit SearchServer(istream& document_input);
  void UpdateDocumentBase(istream& document_input);
  void AddQueriesStream(istream& query_input, ostream& search_results_output);

private:
  InvertedIndex index;
};
