#include "list.h"

#include <stdlib.h>

#include "exception.h"

// make right to be the one after left in the list
static inline void CombineListNode(ListNode *const left,
                                   ListNode *const right) {
  left->nxt = right;
  right->pre = left;
}

List *NewList(const size_t size_of_value) {
  List *list = (List *)malloc(sizeof(List));
  if (!list) Error("Malloc failed in NewList");
  list->size = 0;
  list->dummy_head = (ListNode *)malloc(sizeof(ListNode));
  if (!list->dummy_head) Error("Malloc failed in NewList");
  list->dummy_tail = (ListNode *)malloc(sizeof(ListNode));
  if (!list->dummy_tail) Error("Malloc failed in NewList");
  CombineListNode(list->dummy_head, list->dummy_tail);
  list->dummy_head->pre = list->dummy_head->value = NULL;
  list->dummy_tail->nxt = list->dummy_tail->value = NULL;
  return list;
}

void DeleteList(const List *const list) {
  ClearList((List *const)list);
  free(list->dummy_head);
  free(list->dummy_tail);
  free((void *)list);
}

void ClearList(List *const list) {
  const ListNode *now = list->dummy_head->nxt;
  while (now != list->dummy_tail) now = EraseList(list, now);
  list->size = 0;
}

const ListNode *InsertList(List *const list, ListNode *const pos,
                           void *const value) {
  if (pos == list->dummy_head) Error("Can't insert a node before dummy head");
  ListNode *new_node = (ListNode *)malloc(sizeof(ListNode));
  if (!new_node) Error("Malloc failed in NewList");
  new_node->value = value;
  CombineListNode(pos->pre, new_node);
  CombineListNode(new_node, pos);
  list->size++;
  return new_node;
}

const ListNode *EraseList(List *const list, const ListNode *const node) {
  if (node == list->dummy_head) Error("Can't erase dummy_head");
  if (node == list->dummy_tail) Error("Can't erase dummy_tail");
  const ListNode *ret = node->nxt;
  CombineListNode(node->pre, node->nxt);
  list->size--;
  free(node->value);
  free((void *)node);
  return ret;
}

static inline ListNode *MoveListNode(const List *const list,
                                     const ListNode *const node, int step) {
  ListNode *ret = (ListNode *)node;
  while (ret != list->dummy_tail && step--) ret = ret->nxt;
  return ret;
}

void SortList(const List *const list,
              bool (*cmp)(const void *const lhs, const void *const rhs)) {
  // l is the length of the sublist that need to be combined
  for (int l = 1; l < list->size; l <<= 1) {
    // now is the node where the sorted sublist starts
    // the first sublist ranges between [left_now, left_end)
    // the second sublist ranges between [right_now, right_end)
    ListNode *now, *left_now, *left_end, *right_now, *right_end;
    now = list->dummy_head;
    do {
      // init
      left_now = now->nxt;
      right_now = left_end = MoveListNode(list, left_now, l);
      right_end = MoveListNode(list, right_now, l);
      // merge two sublists
      while (left_now != left_end && right_now != right_end) {
        if (cmp(left_now->value, right_now->value)) {
          // if left_now <= right_now, add left_now to the sorted sublist
          CombineListNode(now, left_now);
          now = now->nxt;
          left_now = left_now->nxt;
        } else {
          // if left_now > right_now, add right_now to the sorted sublist
          CombineListNode(now, right_now);
          now = now->nxt;
          right_now = right_now->nxt;
        }
      }
      if (left_now != left_end) {
        // add the left part of the first sublist to the sorted sublist
        CombineListNode(now, left_now);
        // find the end of the sorted sublist
        while (now->nxt != left_end) now = now->nxt;
      } else if (right_now != right_end) {
        // add the left part of the second sublist to the sorted sublist
        CombineListNode(now, right_now);
        // find the end of the sorted sublist
        while (now->nxt != right_end) now = now->nxt;
      }
      // combine the sorted sublist with the unsorted one
      CombineListNode(now, right_end);
    } while (right_now != list->dummy_tail);
  }
}
