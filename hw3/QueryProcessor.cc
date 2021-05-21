/*
 * Copyright ©2021 Travis McGaha.  All rights reserved.  Permission is
 * hereby granted to students registered for University of Washington
 * CSE 333 for use solely during Spring Quarter 2021 for purposes of
 * the course.  No other use, copying, distribution, or modification
 * is permitted without prior written consent. Copyrights for
 * third-party components of this work must be honored.  Instructors
 * interested in reusing these course materials should contact the
 * author.
 */

#include "./QueryProcessor.h"

#include <iostream>
#include <algorithm>

extern "C" {
  #include "./libhw1/CSE333.h"
}

using std::list;
using std::sort;
using std::string;
using std::vector;

namespace hw3 {

QueryProcessor::QueryProcessor(const list<string> &index_list, bool validate) {
  // Stash away a copy of the index list.
  index_list_ = index_list;
  array_len_ = index_list_.size();
  Verify333(array_len_ > 0);

  // Create the arrays of DocTableReader*'s. and IndexTableReader*'s.
  dtr_array_ = new DocTableReader *[array_len_];
  itr_array_ = new IndexTableReader *[array_len_];

  // Populate the arrays with heap-allocated DocTableReader and
  // IndexTableReader object instances.
  list<string>::const_iterator idx_iterator = index_list_.begin();
  for (int i = 0; i < array_len_; i++) {
    FileIndexReader fir(*idx_iterator, validate);
    dtr_array_[i] = fir.NewDocTableReader();
    itr_array_[i] = fir.NewIndexTableReader();
    idx_iterator++;
  }
}

QueryProcessor::~QueryProcessor() {
  // Delete the heap-allocated DocTableReader and IndexTableReader
  // object instances.
  Verify333(dtr_array_ != nullptr);
  Verify333(itr_array_ != nullptr);
  for (int i = 0; i < array_len_; i++) {
    delete dtr_array_[i];
    delete itr_array_[i];
  }

  // Delete the arrays of DocTableReader*'s and IndexTableReader*'s.
  delete[] dtr_array_;
  delete[] itr_array_;
  dtr_array_ = nullptr;
  itr_array_ = nullptr;
}

// This structure is used to store a index-file-specific query result.
typedef struct {
  DocID_t doc_id;  // The document ID within the index file.
  int rank;       // The rank of the result so far.
} IdxQueryResult;

// Helper function that processes a query against a single index and
// returns a list of IdxQueryResults. If no documents match the query,
// a valid but empty list will be returned.
static list<IdxQueryResult> GetIndexQueryResults(const IndexTableReader *itr, 
                                                 const vector<string> &query);

vector<QueryProcessor::QueryResult>
QueryProcessor::ProcessQuery(const vector<string> &query) const {
  Verify333(query.size() > 0);

  // STEP 1.
  // (the only step in this file)
  vector<QueryProcessor::QueryResult> final_result;
  for(int i = 0; i < array_len_; i++) {
    list<IdxQueryResult> results = GetIndexQueryResults(itr_array_[i], query);
    const DocTableReader *dtr = dtr_array_[i];
    // Accumulate all of the results for the current index.
    for (const auto &result : results) {
      string filename;
      // If this lookup fails, something is unrecoverably wrong.
      Verify333(dtr->LookupDocID(result.doc_id, &filename));

      final_result.push_back({filename, result.rank});
    }
  }

  // Sort the final results.
  sort(final_result.begin(), final_result.end());
  return final_result;
}


static list<IdxQueryResult> GetIndexQueryResults(const IndexTableReader *itr, 
                                                 const vector<string> &query) {
  list<IdxQueryResult> ret_list;
  const DocIDTableReader *dtr = itr->LookupWord(query[0]);
  if (!dtr) {
    // There can't be any matches. Exit early.
    return ret_list;
  }

  // Get the initial matching document list. The starting rank is just
  // the number of occurences.
  list<DocIDElementHeader> matches = dtr->GetDocIDList();
  for (const auto& header : matches) {
    ret_list.push_back({header.doc_id, header.num_positions});
  }

  // Process the rest of the query words.
  for (size_t i = 1; i < query.size(); i++) {
    dtr = itr->LookupWord(query[i]);
    if (!dtr) {
      // Everything will be removed from the result list, so exit early.
      return list<IdxQueryResult>();
    }

    for (auto it = ret_list.begin(); it != ret_list.end(); ) {
      list<DocPositionOffset_t> positions;
      bool found = dtr->LookupDocID(it->doc_id, &positions);

      // Either update or remove the IdxQueryResult at it.
      if (found) {
        it->rank += positions.size();
        it++;
      } else {
        it = ret_list.erase(it);
      }
    }
  }
  return ret_list;
}

}  // namespace hw3
