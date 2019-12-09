#include "search_server.h"
#include <cstdlib>
#include <sstream>
#include <iostream>
#include "profile.h"
using namespace std;

unordered_map<string, vector<int64_t>> LogDuration::metrics;

string generateWord() {
    static const char alphanum[] = "abcdefghijklmnopqrstuvwxyz";

    size_t len = (arc4random() % 15) + 1;

    string res;

    res.reserve(len);

    for (size_t i = 0; i < len; i++) {
        res += alphanum[arc4random() % 26];
    }

    return res;
}

string generateDoc(size_t doc_wc) {
    ostringstream os;

    for (size_t i = 0; i < doc_wc; i++) {
        if (i > 0) {
            os << ' ';
        }

        os << generateWord();
    }

    return os.str();
}

string generateDocs(size_t docs_num, std::size_t doc_wc) {
    ostringstream os;

    for (size_t i = 0; i < docs_num; i++) {
        if (i > 0) {
            os << '\n';
        }

        os << generateDoc(doc_wc);
    }

    return os.str();
}

int main2() {
    istringstream is(generateDocs(800, 1000));

    SearchServer srv;

    {
        LOG_DURATION("Update")
        srv.UpdateDocumentBase(is);
    }

    istringstream queries(generateDocs(300000, 10));

    ostringstream os;

    {
        LOG_DURATION("Search")
        srv.AddQueriesStream(queries, os);
    }

    LogDuration::printMetrics();
}