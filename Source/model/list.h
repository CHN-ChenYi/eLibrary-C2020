#ifndef LIST_H_
#define LIST_H_

#include "genlib.h"  // import type of bool

typedef struct ListNode {
  void *value;
  struct ListNode *pre, *nxt;
} ListNode;

typedef struct List {
  int size;
  ListNode *dummy_head, *dummy_tail;
} List;

// Malloc a list, init it and return a pointer points to it
List *NewList(const size_t size_of_value);

// Free the list
void DeleteList(const List *const list);

// Erases all elements from the list
void ClearList(List *const list);

// Inserts elements before pos, returns a pointer points to the inserted node
// Note that the value may be freed during the operation of list,
// so after inserting the value, you shouldn't use it any more
const ListNode *InsertList(List *const list, ListNode *const pos,
                           void *const value);

// Erases node and returns a pointer points to the node after it
const ListNode *EraseList(List *const list, const ListNode *const node);

// Sorts the list in the specified cmp function which returns whether lhs <= rhs
// Note that iterators remain valid after sorting
// The implementation is merge sort, so it's stable
void SortList(const List *const list,
              bool (*cmp)(const void *const lhs, const void *const rhs));

#endif  // LIST_H_
