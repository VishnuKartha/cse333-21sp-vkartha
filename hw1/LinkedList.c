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

#include <stdio.h>
#include <stdlib.h>

#include "CSE333.h"
#include "LinkedList.h"
#include "LinkedList_priv.h"


///////////////////////////////////////////////////////////////////////////////
// LinkedList implementation.

LinkedList* LinkedList_Allocate(void) {
  // Allocate the linked list record.
  LinkedList *ll = (LinkedList *) malloc(sizeof(LinkedList));
  Verify333(ll != NULL);

  // STEP 1: initialize the newly allocated record structure.
  ll -> head = NULL;
  ll -> tail = NULL;
  ll -> num_elements = 0;
  // Return our newly minted linked list.
  return ll;
}

void LinkedList_Free(LinkedList *list,
                     LLPayloadFreeFnPtr payload_free_function) {
  Verify333(list != NULL);
  Verify333(payload_free_function != NULL);

  // STEP 2: sweep through the list and free all of the nodes' payloads
  // (using the payload_free_function supplied as an argument) and
  // the nodes themselves.

  LinkedListNode* curr = list ->head;
  while (curr != NULL) {
    payload_free_function((void*) curr->payload);
    LinkedListNode* temp = curr;
    curr = curr -> next;
    free(temp);
  }
  // free the LinkedList
  free(list);
}

int LinkedList_NumElements(LinkedList *list) {
  Verify333(list != NULL);
  return list->num_elements;
}

void LinkedList_Push(LinkedList *list, LLPayload_t payload) {
  Verify333(list != NULL);

  // Allocate space for the new node.
  LinkedListNode *ln = (LinkedListNode *) malloc(sizeof(LinkedListNode));
  Verify333(ln != NULL);

  // Set the payload
  ln->payload = payload;

  if (list->num_elements == 0) {
    // Degenerate case; list is currently empty
    Verify333(list->head == NULL);
    Verify333(list->tail == NULL);
    ln->next = ln->prev = NULL;
    list->head = list->tail = ln;
  } else {
    // STEP 3: typical case; list has >=1 elements
    Verify333(list->head != NULL);
    Verify333(list->tail != NULL);
    LinkedListNode* prevHead = list -> head;
    ln -> next = prevHead;
    ln -> prev = NULL;
    list -> head = ln;
    prevHead -> prev = ln;
  }
  list -> num_elements += 1;
}

bool LinkedList_Pop(LinkedList *list, LLPayload_t *payload_ptr) {
  Verify333(payload_ptr != NULL);
  Verify333(list != NULL);
  // pop fails
  if (list->num_elements == 0) return false;

  // STEP 4: implement LinkedList_Pop.  Make sure you test for
  // and empty list and fail.  If the list is non-empty, there
  // are two cases to consider: (a) a list with a single element in it
  // and (b) the general case of a list with >=2 elements in it.
  // Be sure to call free() to deallocate the memory that was
  // previously allocated by LinkedList_Push().
  LinkedListNode* popped = list->head;
  *payload_ptr = popped->payload;
  if (list->num_elements == 1) {
    list->head = NULL;
    list->tail = NULL;
  } else {
     LinkedListNode* newHead = popped->next;
     newHead->prev = NULL;
     list->head = newHead;
  }
  list->num_elements-= 1;
  free(popped);
  // pop succeeded
  return true;
}

void LinkedList_Append(LinkedList *list, LLPayload_t payload) {
  Verify333(list != NULL);

  // STEP 5: implement LinkedList_Append.  It's kind of like
  // LinkedList_Push, but obviously you need to add to the end
  // instead of the beginning.
    // Allocate space for the new node.
  LinkedListNode *ln = (LinkedListNode *) malloc(sizeof(LinkedListNode));
  Verify333(ln != NULL);

  // Set the payload
  ln->payload = payload;

  if (list->num_elements == 0) {
    // Degenerate case; list is currently empty
    Verify333(list->head == NULL);
    Verify333(list->tail == NULL);
    ln->next = ln->prev = NULL;
    list->head = list->tail = ln;
  } else {  // typical case list has 1 or more elements
    Verify333(list->head != NULL);
    Verify333(list->tail != NULL);
    LinkedListNode* prevTail = list -> tail;
    ln -> prev = prevTail;
    ln -> next = NULL;
    list -> tail = ln;
    prevTail -> next = ln;
  }
    list -> num_elements += 1;
}

void LinkedList_Sort(LinkedList *list, bool ascending,
                     LLPayloadComparatorFnPtr comparator_function) {
  Verify333(list != NULL);
  if (list->num_elements < 2) {
    // No sorting needed.
    return;
  }

  // We'll implement bubblesort! Nnice and easy, and nice and slow :)
  int swapped;
  do {
    LinkedListNode *curnode;

    swapped = 0;
    curnode = list->head;
    while (curnode->next != NULL) {
      int compare_result = comparator_function(curnode->payload,
                                               curnode->next->payload);
      if (ascending) {
        compare_result *= -1;
      }
      if (compare_result < 0) {
        // Bubble-swap the payloads.
        LLPayload_t tmp;
        tmp = curnode->payload;
        curnode->payload = curnode->next->payload;
        curnode->next->payload = tmp;
        swapped = 1;
      }
      curnode = curnode->next;
    }
  } while (swapped);
}


///////////////////////////////////////////////////////////////////////////////
// LLIterator implementation.

LLIterator* LLIterator_Allocate(LinkedList *list) {
  Verify333(list != NULL);

  // OK, let's manufacture an iterator.
  LLIterator *li = (LLIterator *) malloc(sizeof(LLIterator));
  Verify333(li != NULL);

  // Set up the iterator.
  li->list = list;
  li->node = list->head;

  return li;
}

void LLIterator_Free(LLIterator *iter) {
  Verify333(iter != NULL);
  free(iter);
}

bool LLIterator_IsValid(LLIterator *iter) {
  Verify333(iter != NULL);
  Verify333(iter->list != NULL);

  return (iter -> node != NULL);
}

bool LLIterator_Next(LLIterator *iter) {
  Verify333(iter != NULL);
  Verify333(iter->list != NULL);
  Verify333(iter->node != NULL);

  // STEP 6: try to advance iterator to the next node and return true if
  // you succeed, false otherwise
  if (iter -> node -> next == NULL) {
     iter -> node = NULL;
     return false;
  } else {
     iter -> node = iter -> node -> next;
     return true;
  }
}

void LLIterator_Get(LLIterator *iter, LLPayload_t *payload) {
  Verify333(iter != NULL);
  Verify333(iter->list != NULL);
  Verify333(iter->node != NULL);

  *payload = iter->node->payload;
}

bool LLIterator_Remove(LLIterator *iter,
                       LLPayloadFreeFnPtr payload_free_function) {
  Verify333(iter != NULL);
  Verify333(iter->list != NULL);
  Verify333(iter->node != NULL);

  // STEP 7: implement LLIterator_Remove.  This is the most
  // complex function you'll build.  There are several cases
  // to consider:
  // - degenerate case: the list becomes empty after deleting.
  // - degenerate case: iter points at head
  // - degenerate case: iter points at tail
  // - fully general case: iter points in the middle of a list,
  //                       and you have to "splice".
  //
  // Be sure to call the payload_free_function to free the payload
  // the iterator is pointing to, and also free any LinkedList
  // data structure element as appropriate.
  LinkedListNode* nodePointer = iter -> node;
  payload_free_function(nodePointer -> payload);
  // case where the list becomes empty after deletion
  if (iter -> list -> num_elements == 1) {
    iter -> list -> head = NULL;
    iter -> list -> tail = NULL;
    iter -> node = NULL;
    iter -> list -> num_elements -= 1;
    free(nodePointer);
    return false;
  } else if (nodePointer -> prev == NULL) {
    // iter is at head (nodePointer points to head)
    iter -> list -> head = nodePointer -> next;
    iter -> list -> head -> prev = NULL;
    iter -> node = iter -> list -> head;
  } else if (nodePointer -> next == NULL) {
    // iter is at the tail (nodePointer points to tail)
     iter -> list -> tail = nodePointer -> prev;
     iter -> list -> tail -> next = NULL;
     iter -> node = iter -> list -> tail;
  } else {  // fully general case case
     nodePointer -> next -> prev = nodePointer -> prev;
     nodePointer -> prev -> next = nodePointer -> next;
     iter -> node = nodePointer -> next;
  }
  iter -> list -> num_elements -= 1;
  free(nodePointer);
  return true;
  }


///////////////////////////////////////////////////////////////////////////////
// Helper functions

bool LinkedList_Slice(LinkedList *list, LLPayload_t *payload_ptr) {
  Verify333(payload_ptr != NULL);
  Verify333(list != NULL);

  // STEP 8: implement LinkedList_Slice.

  Verify333(payload_ptr != NULL);
  Verify333(list != NULL);
  // slice fails
  if (list->num_elements == 0) return false;

  LinkedListNode* sliced = list->tail;
  *payload_ptr = sliced->payload;
  if (list->num_elements == 1) {
    list->head = NULL;
    list->tail = NULL;
  } else {
     LinkedListNode* newTail =  sliced -> prev;
     newTail-> next = NULL;
     list -> tail = newTail;
  }
  list->num_elements-= 1;
  free(sliced);
  // slice succeeded
  return true;
}

void LLIterator_Rewind(LLIterator *iter) {
  iter->node = iter->list->head;
}
