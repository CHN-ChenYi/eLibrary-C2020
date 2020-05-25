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
List *NewList();

// Free the list
// using void Free(void *const value) to free the pointers in the value
void DeleteList(const List *const list, void (*Free)(void *const value));

// Erases all elements from the list
// using void Free(void *const value) to free the pointers in the value
void ClearList(List *const list, void (*Free)(void *const value));

// Inserts elements before pos, returns a pointer points to the inserted node
// Note that after inserting the value, you shouldn't modify it any more
const ListNode *InsertList(List *const list, ListNode *const pos,
                           void *const value);

// Erases node and returns a pointer points to the node after it
// using void Free(void *const value) to free the pointers in the value
const ListNode *EraseList(List *const list, const ListNode *const node,
                          void (*Free)(void *const value));

// Sorts the list in the specified cmp function which returns whether lhs <= rhs
// Note that iterators remain valid after sorting
// The implementation is merge sort, so it's stable
void SortList(const List *const list,
              bool (*cmp)(const void *const lhs, const void *const rhs));

// Unique a sorted list
// Note that iterators remain valid after sorting
// cmp returns whether lhs == rhs
// using void Free(void *const value) to free the pointers in the value
void UniqueList(List *const list,
                bool (*cmp)(const void *const lhs, const void *const rhs),
                void (*Free)(void *const value));

// Deep copy a list
// copy the value using function Duplicate
List *DuplicateList(const List *const list,
                    void *const (*Duplicate)(void *const value));

#endif  // LIST_H_
