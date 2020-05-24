#include "utility.h"

#include <stdarg.h>
#include <stdio.h>
#include <time.h>

#include "basictype.h"
#include "genlib.h"
#include "history.h"
#include "list.h"
#include "model.h"

bool db_open;
FILE *log_file;
extern User user;
extern size_t id_len;
extern List *history_list;

void InitUtility() { fopen_s(&log_file, ".\\eLibrary.log", "a+"); }

void UninitUtility() { fclose(log_file); }

void Log(char *const msg) {
  // get local time
  const time_t cur_time = time(0);
  char *time = asctime(localtime(&cur_time));
  size_t len = strlen(time);
  while (len && (time[len - 1] == '\r' ||
                 time[len - 1] == '\n'))  // delete \n in the string
    time[--len] = '\0';
  // As DST is not always one hour, calculating loacl time zone is expensive
  // so we just display "Local" instead of "UTC+x"
  fprintf(log_file, "[%s Local] %s\n", time, msg);
#ifdef _DEBUG
  printf("[%s Local] %s\n", time, msg);
#endif  // _DEBUG
}

char *MoveInList(ListNode **const node, List *list, int max_size,
                 bool direction, const char *const list_name,
                 const char *const page_name) {
  char *msg = malloc(sizeof(char) *
                     (59 + id_len + strlen(list_name) + strlen(page_name)));
  if (direction) {
    ListNode *new_node = *node;
    for (int i = max_size; i && new_node != list->dummy_tail; i--)
      new_node = new_node->nxt;
    if (new_node == list->dummy_tail) {
      sprintf(
          msg,
          "[Error] [%s] Fail to turn to the next page in List %s of Page %s",
          user.id, list_name, page_name);
    } else {
      *node = new_node;
      sprintf(msg, "[Info] [%s] Turn to the next page in List %s of Page %s",
              user.id, list_name, page_name);
    }
  } else {
    ListNode *new_node = *node;
    for (int i = max_size; i && new_node != list->dummy_head; i--)
      new_node = new_node->pre;
    if (new_node == list->dummy_head) {
      sprintf(
          msg,
          "[Error] [%s] Fail to turn to the prev page in List %s of Page %s",
          user.id, list_name, page_name);
    } else {
      *node = new_node;
      sprintf(msg, "[Info] [%s] Turn to the prev page in List %s of Page %s",
              user.id, list_name, page_name);
    }
  }
  return msg;
}

bool ErrorHandle(int errno_, int num, ...) {
  if (errno_ == DB_SUCCESS) return FALSE;

  // 处理可变参数
  va_list valist;
  va_start(valist, num);
  while (num--) {
    // 如果错误码和可变参数中的某个吻合，则当作数据库操作成功
    if (errno_ == va_arg(valist, int)) return FALSE;
  }

  char *msg = malloc(sizeof(char) * (41 + id_len));
  switch (errno_) {
    case DB_NOT_FOUND:
      sprintf(msg, "[Error] [%s] DB_NOT_FOUND", user.id);
      break;
    case DB_NOT_OPEN:
      sprintf(msg, "[Error] [%s] DB_NOT_OPEN", user.id);
      break;
    case DB_NOT_CLOSE:
      sprintf(msg, "[Error] [%s] DB_NOT_CLOSE", user.id);
      break;
    case DB_NOT_EXISTS:
      sprintf(msg, "[Error] [%s] DB_NOT_EXISTS", user.id);
      break;
    case DB_FAIL_ON_INIT:
      sprintf(msg, "[Error] [%s] DB_FAIL_ON_INIT", user.id);
      break;
    case DB_FAIL_ON_FETCHING:
      sprintf(msg, "[Error] [%s] DB_FAIL_ON_FETCHING", user.id);
      break;
    case DB_FAIL_ON_WRITING:
      sprintf(msg, "[Error] [%s] DB_FAIL_ON_WRITING", user.id);
      break;
    case DB_FAIL_ON_CREATE:
      sprintf(msg, "[Error] [%s] DB_FAIL_ON_CREATE", user.id);
      break;
    case DB_FAIL_ON_UPDATE:
      sprintf(msg, "[Error] [%s] DB_FAIL_ON_UPDATE", user.id);
      break;
    case DB_FAIL_ON_DELETE:
      sprintf(msg, "[Error] [%s] DB_FAIL_ON_DELETE", user.id);
      break;
    case DB_FAIL_ON_GETTING_PROPERTIES:
      sprintf(msg, "[Error] [%s] DB_FAIL_ON_GETTING_PROPERTIES", user.id);
      break;
    case DB_ALREADY_EXISTS:
      sprintf(msg, "[Error] [%s] DB_ALREADY_EXISTS", user.id);
      break;
    case DB_ENTRY_EMPTY:
      sprintf(msg, "[Error] [%s] DB_ENTRY_EMPTY", user.id);
      break;
    case DB_FAIL:
      sprintf(msg, "[Error] [%s] DB_FAIL", user.id);
      break;
    default:
      Log("[Debug] Unknown errno");
      Error("Unknown errno");
  }
  ReturnHistory(history_list->dummy_tail->pre, msg);
  return TRUE;
}

bool InitCheck(bool no_user) {
  if (!db_open) {  // 检查数据库是否打开
    char *msg = malloc(sizeof(char) * (39 + id_len));
    sprintf(msg, "[Error] [%s] Please open a library first", user.id);
    ReturnHistory(history_list->dummy_tail->pre, msg);
    return TRUE;
  }
  if (!no_user && !user.verified) {  // 检查当前登录用户是否有效
    char *msg = malloc(sizeof(char) * (28));
    sprintf(msg, "[Error] Please log in first");
    ReturnHistory(history_list->dummy_tail->pre, msg);
    return TRUE;
  }
  return FALSE;
}

bool CmpGreaterBorrowRecordByReturnTime(const void *const lhs,
                                        const void *const rhs) {
  return strcmp(((BorrowRecord *)lhs)->returned_date,
                ((BorrowRecord *)rhs)->returned_date) >= 0;
}

bool CmpLessBorrowRecordByReturnTime(const void *const lhs,
                                     const void *const rhs) {
  return strcmp(((BorrowRecord *)lhs)->returned_date,
                ((BorrowRecord *)rhs)->returned_date) <= 0;
}

bool CmpLessBookById(const void *const lhs, const void *const rhs) {
  return strcmp(((Book *)lhs)->id, ((Book *)rhs)->id) <= 0;
}

bool CmpLessBookByTitle(const void *const lhs, const void *const rhs) {
  return strcmp(((Book *)lhs)->title, ((Book *)rhs)->title) <= 0;
}

bool CmpLessBookByAuthor(const void *const lhs, const void *const rhs) {
  return strcmp(((Book *)lhs)->authors[0], ((Book *)rhs)->authors[0]) <= 0;
}

void *const StrCpy(void *const str) {
  char *ret = malloc(sizeof(char) * (strlen(str) + 1));
  strcpy(ret, str);
  return ret;
}

bool StrLess(const void *const lhs, const void *rhs) {
  return strcmp(lhs, rhs) <= 0;
}

bool StrSame(const void *const lhs, const void *rhs) {
  return strcmp(lhs, rhs) == 0;
}
