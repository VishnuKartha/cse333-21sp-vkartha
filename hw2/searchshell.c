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

// Needed to use getline without compiler implicit declaration warning
#define _GNU_SOURCE

#include <stdbool.h>
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

// Maximum number of words in a query.
// See https://edstem.org/us/courses/4899/discussion/390660.
#define MAX_QUERY_LEN 256

// Prints out the usage of the program and exits.
static void Usage(void);

// Tries to read the next line in f into ret_str. Returns true if something
// was successfully read. A false return value indicates an error or EOF; if
// errno is zero EOF was reached, else there was an error.
//
// Due to the nature of getline(), *ret_str should be NULL before invoking this
// function. Regardless of the return value, the caller is responsible for
// freeing *ret_str if it is not NULL.
static bool GetNextLine(FILE *f, char **ret_str);

// Splits a line into words and populates tokens with at most
// max_tokens pointers to the beginning of each token. Caller does not
// need to free the tokens. Converts all alphabetic characters to lowercase.
// Returns the number of tokens found.
static int BuildQuery(char **tokens, char *line, int max_tokens);

// Performs a query on the given index and DocTable. Prints out the results
// to the given file with a line for each file containing every query word
// in the format "  filepath (rank)". The lines are sorted in non-increasing
// order by rank.
static void ProcessQuery(FILE *f, DocTable *dt, MemIndex *mi,
                         char **query, int query_len);


//////////////////////////////////////////////////////////////////////////////
//
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
  printf("Indexing '%s'...\n", argv[1]);
  bool build_result = CrawlFileTree(argv[1], &dt, &index);
  if (!build_result) {
    fprintf(stderr, "Failed to crawl directory at %s\n", argv[1]);
    return EXIT_FAILURE;
  }

  // Read and query until the user signals EOF or an error is encountered.
  while (true) {
    // Prompt for query from stdout and read query from stdin
    printf("enter query:\n");

    char *line = NULL;
    if (!GetNextLine(stdin, &line)) {
      if (line != NULL) {
        free(line);
      }
      if (errno == 0) {
        // This was just EOF; don't report an error.
        break;
      }
      perror("Couldn't read query from stdin\n");
      DocTable_Free(dt);
      MemIndex_Free(index);
      return EXIT_FAILURE;
    }

    // Split query line into tokens.
    char *query[MAX_QUERY_LEN];
    int query_len = BuildQuery(query, line, MAX_QUERY_LEN);

    // Perform the query and output to stdout.
    ProcessQuery(stdout, dt, index, query, query_len);

    // Clean up.
    free(line);
  }

  printf("Cleaning up...\n");
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

static bool GetNextLine(FILE *f, char **ret_str) {
  size_t len = 0;
  ssize_t line_size = getline(ret_str, &len, stdin);
  return line_size >= 0;
}

static int BuildQuery(char **tokens, char *line, int max_tokens) {
  int num_tokens = 0;
  char *saveptr;

  // Use strtok_r to grab one token at a time.
  char *tok = strtok_r(line, " \n", &saveptr);
  while (tok != NULL) {
    // Convert to lowercase.
    char *cur = tok;
    while (*cur != '\0') {
      if (isalpha(*cur)) {
        *cur = tolower(*cur);
      }
      cur++;
    }

    tokens[num_tokens] = tok;
    num_tokens++;
    tok = strtok_r(NULL, " \n", &saveptr);
  }
  return num_tokens;
}

static void ProcessQuery(FILE *f, DocTable *dt, MemIndex *mi,
                         char **query, int query_len) {
  LinkedList *results = MemIndex_Search(mi, query, query_len);
  if (results != NULL) {
    LLIterator *it = LLIterator_Allocate(results);
    // Iterate through the search results, reporting the associated document
    // path and rank.
    while (LLIterator_IsValid(it)) {
      SearchResult *sr;
      LLIterator_Get(it, (LLPayload_t *) &sr);

      char *path = DocTable_GetDocName(dt, sr->doc_id);
      fprintf(f, "  %s (%d)\n", path, sr->rank);
      LLIterator_Next(it);
    }
    LLIterator_Free(it);
    LinkedList_Free(results, free);
  }
}
