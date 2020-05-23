#ifndef HISTORY_H_
#define HISTORY_H_

#include "gui.h"
#include "list.h"

#define HISTORY_MAX 100

typedef struct History {
  Page page;
  State state;
} History;

void InitHistory();
void UninitHistory();

History *const TopHistory();
void PushBackHistory(History *const new_history);
void PopBackHistory();
void ClearHistory();
void FreeHistory(void *const history_);
void ReturnHistory(ListNode *go_back_to, char *msg);

#endif  // !HISTORY_H_
