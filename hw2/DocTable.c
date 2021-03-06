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
#include "./DocTable.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "libhw1/CSE333.h"
#include "libhw1/HashTable.h"

#define HASHTABLE_INITIAL_NUM_BUCKETS 2

// Helper function for freeing the values in a HashTable.
static void TableValue_Free(HTValue_t value);

// This structure represents a DocTable; it contains two hash tables, one
// mapping from document id to documemnt name, and one mapping from
// document name to document id.
struct doctable_st {
  HashTable *id_to_name;  // mapping document id to document name
  HashTable *name_to_id;  // mapping document name to document id
  DocID_t    max_id;            // max doc ID allocated so far
};

DocTable *DocTable_Allocate(void) {
  DocTable *dt = (DocTable *) malloc(sizeof(DocTable));

  dt->id_to_name = HashTable_Allocate(HASHTABLE_INITIAL_NUM_BUCKETS);
  dt->name_to_id = HashTable_Allocate(HASHTABLE_INITIAL_NUM_BUCKETS);
  dt->max_id = 1;  // we reserve max_id = 0 for the invalid docID

  return dt;
}

void DocTable_Free(DocTable *table) {
  Verify333(table != NULL);

  // STEP 1.
  HashTable_Free(table->id_to_name, TableValue_Free);
  HashTable_Free(table->name_to_id, TableValue_Free);
  free(table);
}

int DocTable_NumDocs(DocTable *table) {
  Verify333(table != NULL);
  return HashTable_NumElements(table->id_to_name);
}

DocID_t DocTable_Add(DocTable *table, char *doc_name) {
  char *doc_copy;
  DocID_t *doc_id;
  HTKeyValue_t kv, old_kv;

  Verify333(table != NULL);

  // STEP 2.
  // Check to see if the document already exists. Then make a copy of the
  // doc_name and allocate space for the new ID.
  HTKey_t hashed_doc_name =
    (HTKey_t) FNVHash64((unsigned char *) doc_name, strlen(doc_name));

  if (HashTable_Find(table->name_to_id, hashed_doc_name, &old_kv)) {
    // The document already exists, so just return the existing ID.
    return *((DocID_t *) old_kv.value);
  } else {
    // We need to add a new document. Prepare space for the document
    // name and ID.
    doc_copy = (char *) malloc(strlen(doc_name) + 1);
    Verify333(doc_copy != NULL);
    snprintf(doc_copy, strlen(doc_name) + 1, "%s", doc_name);
    Verify333(strcmp(doc_copy, doc_name) == 0);
    doc_id = (DocID_t *) malloc(sizeof(DocID_t));
    Verify333(doc_id != NULL);
  }

  *doc_id = table->max_id;
  table->max_id++;

  // STEP 3.
  // Set up the key/value for the id->name mapping, and do the insert.
  kv.key = (HTKey_t) *doc_id;
  kv.value = (HTValue_t) doc_copy;
  HashTable_Insert(table->id_to_name, kv, &old_kv);

  // STEP 4.
  // Set up the key/value for the name->id, and/ do the insert.
  // Be careful about how you calculate the key for this mapping.
  // You want to be sure that how you do this is consistent with
  // the provided code.

  kv.key = hashed_doc_name;
  kv.value = (HTValue_t) doc_id;
  HashTable_Insert(table->name_to_id, kv, &old_kv);

  return *doc_id;
}

DocID_t DocTable_GetDocID(DocTable *table, char *doc_name) {
  HTKey_t key;
  HTKeyValue_t kv;

  Verify333(table != NULL);
  Verify333(doc_name != NULL);

  // STEP 5.
  // Try to find the passed-in doc in name_to_id table.
  key = (HTKey_t) FNVHash64((unsigned char *) doc_name, strlen(doc_name));
  if (HashTable_Find(table->name_to_id, key, &kv)) {
    return *((DocID_t *) kv.value);
  } else {
    return INVALID_DOCID;
  }
}

char *DocTable_GetDocName(DocTable *table, DocID_t doc_id) {
  HTKeyValue_t kv;

  Verify333(table != NULL);
  Verify333(doc_id != INVALID_DOCID);

  // STEP 6.
  // Lookup the doc_id in the id_to_name table,
  // and either return the string (i.e., the (char *)
  // saved in the value field for that key) or
  // NULL if the key isn't in the table.
  if (HashTable_Find(table->id_to_name, (HTKey_t) doc_id, &kv)) {
    return (char *) kv.value;
  } else {
    return NULL;
  }
}

HashTable *DT_GetIdToNameTable(DocTable *table) {
  Verify333(table != NULL);
  return table->id_to_name;
}

HashTable *DT_GetNameToIdTable(DocTable *table) {
  Verify333(table != NULL);
  return table->name_to_id;
}

// helper functions

static void TableValue_Free(HTValue_t value) {
  Verify333(value != NULL);
  free(value);
}
