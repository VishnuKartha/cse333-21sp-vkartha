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

// Feature test macro for strtok_r (c.f., Linux Programming Interface p. 63)
#define _XOPEN_SOURCE 600

// Needed to use getline
#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <errno.h>

#include "libhw1/CSE333.h"
#include "libhw1/LinkedList.h"
#include "./CrawlFileTree.h"
#include "./DocTable.h"
#include "./MemIndex.h"

//////////////////////////////////////////////////////////////////////////////
// Helper function declarations, constants, etc

#define MAX_QUERY_LEN 256

static void Usage(void);
static void ProcessQueries(DocTable *dt, MemIndex *mi);
static int GetNextLine(FILE *f, char **ret_str);


//////////////////////////////////////////////////////////////////////////////
// Main
int main(int argc, char **argv) {
  if (argc != 2) {
    Usage();
  }

  // Implement searchshell!  We're giving you very few hints
  // on how to do it, so you'll need to figure out an appropriate
  // decomposition into functions as well as implementing the
  // functions.  There are several major tasks you need to build:
  //
  //  - Crawl from a directory provided by argv[1] to produce and index
  //  - Prompt the user for a query and read the query from stdin, in a loop
  //  - Split a query into words (check out strtok_r)
  //  - Process a query against the index and print out the results
  //
  // When searchshell detects end-of-file on stdin (cntrl-D from the
  // keyboard), searchshell should free all dynamically allocated
  // memory and any other allocated resources and then exit.

  // Crawl from directory at argv[1] to build index.
  DocTable *dt;
  MemIndex *index;
  bool build_result = CrawlFileTree(argv[1], &dt, &index);
  if (!build_result) {
    fprintf(stderr, "Failed to crawl directory at %s\n", argv[1]);
    return EXIT_FAILURE;
  }

  while (true) {
    // Prompt for query and read query from stdin
    printf("enter query:\n");
    char *line = NULL;
    size_t len = 0;
    ssize_t line_size = getline(&line, &len, stdin);
    if (line_size == -1) {
      free(line);
      if (errno == 0) {
        break;
      }
      perror("Couldn't read query from stdin\n");
      DocTable_Free(dt);
      MemIndex_Free(index);
      return EXIT_FAILURE;
    }

    // Split query line into words.
    char *tokens[MAX_QUERY_LEN];
    int query_len = 0;
    char *saveptr;
    char *tok = strtok_r(line, " \n", &saveptr);
    while (tok != NULL) {
      char *cur = tok;
      while (*cur != '\0') {
        if (isalpha(*cur)) {
          *cur = tolower(*cur);
        }
        cur++;
      }
      tokens[query_len] = tok;
      query_len++;
      tok = strtok_r(NULL, " \n", &saveptr);
    }

    // Perform the query.
    LinkedList *results = MemIndex_Search(index, tokens, query_len);
    if (results != NULL) {
      LLIterator *it = LLIterator_Allocate(results);
      while (LLIterator_IsValid(it)) {
        SearchResult *sr;
        LLIterator_Get(it, (LLPayload_t *) &sr);
        char *path = DocTable_GetDocName(dt, sr->doc_id);
        printf("  %s (%d)\n", path, sr->rank);
        LLIterator_Next(it);
      }
      LLIterator_Free(it);
      LinkedList_Free(results, free);
    }

    free(line);
  }

  DocTable_Free(dt);
  MemIndex_Free(index);
  
  return EXIT_SUCCESS;
}


//////////////////////////////////////////////////////////////////////////////
// Helper function definitions

static void Usage(void) {
  fprintf(stderr, "Usage: ./searchshell <docroot>\n");
  fprintf(stderr,
          "where <docroot> is an absolute or relative " \
          "path to a directory to build an index under.\n");
  exit(EXIT_FAILURE);
}

static void ProcessQueries(DocTable *dt, MemIndex *mi) {
}

static int GetNextLine(FILE *f, char **ret_str) {
  return -1;  // you may want to change this
}
