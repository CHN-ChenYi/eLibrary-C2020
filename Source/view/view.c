#include "view.h"

#include <io.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdarg.h>

#include "basictype.h"
#include "exception.h"
#include "extgraph.h"
#include "graphics.h"
#include "gui.h"
#include "hash.h"
#include "list.h"
#include "model.h"

typedef struct History {
  Page page;
  State state;
} History;

static List *history_list;
static User user;
static char lib_path[MAX_PATH + 1], program_path[MAX_PATH + 1];
static bool db_open;
static size_t lib_path_len, id_len, program_path_len;
static char book_db_dir[MAX_PATH + 1], user_db_dir[MAX_PATH + 1],
    borrowrecord_db_dir[MAX_PATH + 1];
static FILE *log_file;
static LibImage edit_cover, unknown_cover;
static inline void Log(char *const msg);
static inline char *MoveInList(ListNode **const node, List *list, int max_size,
                               bool direction, const char *const list_name,
                               const char *const page_name);
// FALSE for success，num 是可变参数的个数，可变参数是可以接受的报错
static inline bool ErrorHandle(int errno_, int num, ...);
// FALSE for success, no_user 为 true 表示可以接受未登录用户
static inline bool InitCheck(bool no_user);
static void FreeHistory(void *const history_);
static inline History *const TopHistory();
static inline void PushBackHistory(History *const new_history);
static inline void PopBackHistory();
static inline void ClearHistory();
static inline void ReturnHistory(ListNode *go_back_to, char *msg);
static void BookSearch_BorrowCallback(Book *book);
static void BookSearch_BookCallback(Book *book);
static void inline BookSearchDisplay(char *keyword, char *msg);
static void BookSearch_SearchCallback(char *keyword);
static void BookSearch_TurnPage(bool direction);
static void LendAndBorrow_SearchCallback(char *keyword);
static void LendAndBorrow_ReturnCallback(ListNode *book,
                                         ListNode *borrow_record);
static void LendAndBorrow_TurnPage(bool direction);
static void UserModify_ConfirmCallback();
static void UserModify_TurnPage(bool direction);
static bool CmpGreaterBorrowRecordByReturnTime(const void * const lhs, const void * const rhs);
static void inline UserSearchInfoDisplay(User *show_user, char *msg);
static void UserSearch_InfoCallback(User *user);
static void inline UserSearchDisplay(char *keyword, char *msg);
static void UserSearch_SearchCallback(char *keyword);
static void UserSearch_TurnPage(bool direction);
static void LoginOrRegister_LoginCallback();
static void UserManagement_ApproveCallback(ListNode *user_node, bool approve);
static void UserManagement_DeleteCallback(ListNode *user_node);
static void UserManagement_TurnPage(bool direction, bool type);
static void BookDisplayAdminDisplay(char *msg);
static void BookDisplay_AdminCallback();
static void BookDisplay_CoverCallback();
static void BookDisplay_ConfirmCallback();
static void BookDisplay_DeleteCallback();
static void BookDisplay_BorrowCallback();
static void BookDisplay_CopyPasteCallback();
static void BorrowDisplay_TurnPage(bool direction);
static void Library_BookCallback(ListNode *book);
static bool CmpById(const void *const lhs, const void *const rhs);
static bool CmpByTitle(const void *const lhs, const void *const rhs);
static bool CmpByAuthor(const void *const lhs, const void *const rhs);
static void Library_SortCallback(SortKeyword sort_keyword);
static void Library_SwitchCallback();
static void Library_TurnPage(bool direction);
static void *const StrCpy(void *const str);
static void Statistics_SelectCallback(ListNode *catalog);
static void Statistics_TurnPage(bool direction, bool type);
static bool CmpLessBorrowRecordByReturnTime(const void *const lhs,
                                            const void *const rhs);
static inline void Navigation_LendAndBorrow(char *msg);
static inline void Navigation_BookSearch(char *msg);
static inline void Navigation_UserSearch(char *msg);
static inline void Navigation_ManualOrAbout(bool type, char *msg);
static inline void Navigation_UserLogInOrRegister(bool type, char *msg);
static inline void Navigation_UserLogOut(char *msg);
static inline void Navigation_UserModify(char *msg);
static inline void Navigation_UserManagement(char *msg);
static inline void Navigation_Library(char *msg);
static inline void Navigation_OpenOrInitLibrary(bool type, char *msg);
static inline void Navigation_SaveLibrary(bool type, char *msg);
static void Navigation_BookDisplayOrInit(Book *book, bool type, char *msg);
static inline void Navigation_BookInit(char *msg);
static bool StrLess(const void *const lhs, const void *rhs);
static bool StrSame(const void *const lhs, const void *rhs);
static inline void Navigation_Statistics(char *msg);
static inline void Navigation_Return(char *msg);
static inline void Navigation_Exit();
extern void NavigationCallback(Page nav_page);

void InitView() {
  // init history
  history_list = NewList();
  History *const new_history = malloc(sizeof(History));
  new_history->page = kWelcome;
  PushBackHistory(new_history);

  // init log
  fopen_s(&log_file, ".\\eLibrary.log", "a+");

  // load resource
  try {
    loadImage(".\\Resource\\edit_cover.jpg", &edit_cover);
    loadImage(".\\Resource\\unknown_cover.jpg", &unknown_cover);
    except(ErrorException) {
      Log("[Debug] Fail to load resources");
      exit(1);
    }
  } endtry;

  // set up welcome page
  char *msg = malloc(sizeof(char) * 13);
  sprintf(msg, "[Info] Start");
  Log(msg);
  DrawUI(kWelcome, &user, NULL, msg);
}

static inline void Log(char *const msg) {
  // get local time
  time_t cur_time = time(0);
  char *time = asctime(localtime(&cur_time));
  size_t len = strlen(time);
  while (len && (time[len - 1] == '\r' ||
                 time[len - 1] == '\n'))  // delete \n in the string
    time[--len] = '\0';
  // As DST is not always one hour, calculating loacl time zone is expensive
  // so we just display "Local" instead of "UTC+x"
  fprintf(log_file, "[%s Local] %s\n", time, msg);
  printf("[%s Local] %s\n", time, msg); // TODO:(TO/GA) Delete it
}

static inline char *MoveInList(ListNode **const node, List *list, int max_size,
                               bool direction, const char *const list_name,
                               const char *const page_name) {
  char *msg = malloc(sizeof(char) * (59 + id_len + strlen(list_name) +
                                     strlen(page_name)));
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

static inline bool ErrorHandle(int errno_, int num, ...) {
  if (errno_ == DB_SUCCESS)
    return FALSE;

  // 处理可变参数
  va_list valist;
  va_start(valist, num);
  while (num--) {
    if (errno_ == va_arg(valist, int))
      return FALSE;
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

static inline bool InitCheck(bool no_user) {
  if (!db_open) {
    char *msg = malloc(sizeof(char) * (39 + id_len));
    sprintf(msg, "[Error] [%s] Please open a library first", user.id);
    ReturnHistory(history_list->dummy_tail->pre, msg);
    return TRUE;
  }
  if (!no_user && !user.verified) {
    char *msg = malloc(sizeof(char) * (28));
    sprintf(msg, "[Error] Please log in first");
    ReturnHistory(history_list->dummy_tail->pre, msg);
    return TRUE;
  }
  return FALSE;
}

static inline History *const TopHistory() {
  return (History *)history_list->dummy_tail->pre->value;
}

static inline void PushBackHistory(History *const new_history) {
  InsertList(history_list, history_list->dummy_tail, new_history);
  // if the number of history exceed max_size, delete some of the histories
  while (history_list->size > HISTORY_MAX)
    EraseList(history_list, history_list->dummy_head->nxt, FreeHistory);
}

static inline void PopBackHistory() {
  EraseList(history_list, history_list->dummy_tail->pre, FreeHistory);
}

static inline void ClearHistory() { ClearList(history_list, FreeHistory); }

static void FreeHistory(void *const history_) {
  History *const history = history_;
  switch (history->page) {
    case kWelcome:  // 欢迎界面
      break;
    case kLendAndBorrow:  // 借还书
      DeleteList(history->state.lend_and_borrow->books, free);
      DeleteList(history->state.lend_and_borrow->borrow_records, free);
      break;
    case kBookSearch:  // 图书搜索
      free(history->state.book_search->keyword);
      DeleteList(history->state.book_search->book_result, free);
      break;
    case kUserSearch:  // 用户搜索（管理员）
      free(history->state.user_search->keyword);
      DeleteList(history->state.user_search->user_result, free);
      break;
    case kManual:  // 帮助
    case kAbout:   // 关于
      break;
    case kUserRegister:  // 用户注册
    case kUserLogIn:     // 用户登陆
      break;
    // case kLogout:  // 用户登出
    // break;
    case kUserModify:  // 用户信息修改
      DeleteList(history->state.user_modify->borrowrecords, free);
      break;
    case kUserManagement:  // 用户删除/审核（管理员）
      DeleteList(history->state.user_management->to_be_verified, free);
      DeleteList(history->state.user_management->users, free);
      break;
    case kLibrary:  // 图书库显示
      DeleteList(history->state.library->books, free);
      DeleteList(history->state.library->book_covers, free);
      break;
    // case kInitLibrary:  // 图书库新建
    // break;
    // case kOpenLibrary:  // 图书库打开
    // break;
    // case kSaveLibrary:  // 图书库保存
    // break;
    case kBookDisplay:  // 图书显示
      break;
    case kBookInit:    // 图书新增
    case kBookModify:  // 图书修改/删除
      free(history->state.book_display->book);
      break;
    case kBorrowDisplay:  // 借还书统计（管理员）
      free(history->state.borrow_display->book_id);
      DeleteList(history->state.borrow_display->borrow_record, free);
      break;
    case kStatistics:  // 统计
      DeleteList(history->state.statistics->catalogs, free);
      DeleteList(history->state.statistics->borrow_record, free);
      break;
    // case kReturn:  // 回到上一个界面
    // break;
    // case kExit:    // 关闭程序
    // break;
    default:
      Log("[Debug] Unknown page in FreeHistory");
      Error("Unknown nav_page");
  }
  free(history);
}

static void BookSearch_BorrowCallback(Book *book) {
  if (!book->number_on_the_shelf) {
    char *msg =
        malloc(sizeof(char) * (38 + id_len + strlen(book->id)));
    sprintf(msg, "[Error] [%s] There's no [%s] on the shelf", user.id,
            book->id);
    ReturnHistory(history_list->dummy_tail->pre, msg);
    return;
  }
  book->number_on_the_shelf--;
  if (ErrorHandle(Update(book, book->uid, BOOK), 0)) return;

  BorrowRecord new_record;
  strcpy(new_record.book_id, book->id);
  new_record.book_status = BORROWED;
  new_record.book_uid = book->uid;
  time_t now_time_t = time(0);
  struct tm *now_tm = localtime(&now_time_t);
  sprintf(new_record.borrowed_date, "%04d%02d%02d", now_tm->tm_year + 1900,
          now_tm->tm_mon + 1, now_tm->tm_mday);
  time_t nxt_time_t =
      now_time_t + (time_t)86400 * book->available_borrowed_days;
  struct tm *nxt_tm = localtime(&nxt_time_t);
  sprintf(new_record.returned_date, "%04d%02d%02d", nxt_tm->tm_year + 1900,
          nxt_tm->tm_mon + 1, nxt_tm->tm_mday);
  if (ErrorHandle(GetNextPK(BORROWRECORD, &new_record.uid), 0)) {
    book->number_on_the_shelf++;
    if (ErrorHandle(Update(book, book->uid, BOOK), 0)) return;
    return;
  }
  strcpy(new_record.user_id, user.id);
  new_record.user_uid = user.uid;
  if (ErrorHandle(Create(&new_record, BORROWRECORD), 0)) {
    book->number_on_the_shelf++;
    if (ErrorHandle(Update(book, book->uid, BOOK), 0)) return;
    return;
  }

  char *msg = malloc(sizeof(char) * (25 + id_len + strlen(book->id)));
  sprintf(msg, "[Info] [%s] Borrow book [%s]", user.id, book->id);
  ReturnHistory(history_list->dummy_tail->pre, msg);
}

static void BookSearch_BookCallback(Book *book) {
  Navigation_BookDisplayOrInit(book, 0, NULL);
}

static void inline BookSearchDisplay(char *keyword, char *msg) {
  List *results = NewList();
  if (ErrorHandle(Filter(results, keyword, BOOK), 0)) return;

  History *const new_history = malloc(sizeof(History));
  new_history->page = kBookSearch;
  new_history->state.book_search = malloc(sizeof(BookSearch));
  new_history->state.book_search->keyword = keyword;
  new_history->state.book_search->borrow_callback = BookSearch_BorrowCallback;
  new_history->state.book_search->book_callback = BookSearch_BookCallback;
  new_history->state.book_search->search_callback = BookSearch_SearchCallback;
  new_history->state.book_search->turn_page = BookSearch_TurnPage;
  new_history->state.book_search->book_result = results;
  new_history->state.book_search->book_result_start = results->dummy_head->nxt;
  PushBackHistory(new_history);

  if (!msg) {
    msg = malloc(sizeof(char) * (23 + id_len + strlen(keyword)));
    sprintf(msg, "[Info] [%s] Book search %s", user.id, keyword);
  }
  Log(msg);
  DrawUI(kBookSearch, &user, new_history->state.book_search, msg);
}

static void BookSearch_SearchCallback(char *keyword) {
  char *new_keyword = malloc(sizeof(char) * (strlen(keyword) + 1));
  strcpy(new_keyword, keyword);
  BookSearchDisplay(new_keyword, NULL);
}

static void BookSearch_TurnPage(bool direction) {
  BookSearch *state = TopHistory()->state.book_search;
  char *msg = MoveInList(&state->book_result_start, state->book_result,
                         kBookSearchMax, direction, "Result", "Book Search");
  DrawUI(kBookSearch, &user, state, msg);
}

static void LendAndBorrow_SearchCallback(char *keyword) {
  // 由于 BookSearch 的 Callback 里面已经有了 keyword
  // 的深拷贝，所以这里浅拷贝就可以了
  BookSearch_SearchCallback(keyword);
}

static void LendAndBorrow_ReturnCallback(ListNode *book,
                                         ListNode *borrow_record) {
  Book *returned_book = (Book *)book->value;
  BorrowRecord *returned_borrow_record = (BorrowRecord *)borrow_record->value;

  returned_book->number_on_the_shelf++;
  if (ErrorHandle(Update(returned_book, returned_book->uid, BOOK), 0)) return;

  char *msg = malloc(sizeof(char) * (49 + strlen(returned_book->id) + 16));
  sprintf(msg, "[Info] [%s] Return book [%s], expected return date[%s]",
          user.id, returned_book->id,
          returned_borrow_record->returned_date);

  returned_borrow_record->book_status = RETURNED;
  time_t now_time_t = time(0);
  struct tm *now_tm = localtime(&now_time_t);
  sprintf(returned_borrow_record->returned_date, "%04d%02d%02d",
          now_tm->tm_year + 1900, now_tm->tm_mon + 1, now_tm->tm_mday);
  if (ErrorHandle(Update(returned_borrow_record, returned_borrow_record->uid,
                         BORROWRECORD),
                  0)) {
    free(msg);
    return;
  }

  ReturnHistory(history_list->dummy_tail->pre, msg);
}

static void LendAndBorrow_TurnPage(bool direction) {
  LendAndBorrow *state = TopHistory()->state.lend_and_borrow;
  char *msg =
      MoveInList(&state->borrow_records_start, state->borrow_records,
                 kLendAndBorrowMax, direction, "Record", "Lend And Borrow");
  DrawUI(kLendAndBorrow, &user, state, msg);
}

static void UserModify_ConfirmCallback() {
  User *modified_user = TopHistory()->state.user_modify->user;
  if (user.whoami != ADMINISTRATOR || modified_user->whoami == ADMINISTRATOR) {
    char pwd_type[59];
    sprintf(pwd_type, "%s%s", TopHistory()->state.user_modify->old_password,
            modified_user->salt);
    uint32_t sha_type[8];
    Sha256Sum(sha_type, pwd_type, strlen(pwd_type));
    if (memcmp(sha_type, modified_user->password, sizeof(sha_type))) {
      char *msg = malloc(sizeof(char) * (56 + id_len));
      sprintf(msg, "[Error] [%s] Password incorrect. Can't modify user's info",
              user.id);
      ReturnHistory(history_list->dummy_tail->pre, msg);
      return;
    }
  }
  if (TopHistory()->state.user_modify->new_password[0] != '\0') {
    if (strcmp(TopHistory()->state.user_modify->new_password,
               TopHistory()->state.user_modify->repeat_password)) {
      char *msg = malloc(sizeof(char) * (58 + id_len));
      sprintf(msg,
              "[Error] [%s] Repeat new password doesn't match new password",
              user.id);
      ReturnHistory(history_list->dummy_tail->pre, msg);
      return;
    }
    char new_pwd[59];
    sprintf(new_pwd, "%s%s", TopHistory()->state.user_modify->new_password,
            modified_user->salt);
    Sha256Sum(modified_user->password, new_pwd, strlen(new_pwd));
  }

  List *users = NewList();
  char *query = malloc(sizeof(char) * (10 + strlen(modified_user->id)));
  sprintf(query, "id=%s", modified_user->id);
  free(query);
  if (users->size > 1 ||
      (users->size == 1 &&
       ((User *)users->dummy_head->nxt->value)->uid != modified_user->uid)) {
    DeleteList(users, free);
    char *msg = malloc(sizeof(char) * 42);
    sprintf(msg, "[Error] Fail to modify, id exists");
    ReturnHistory(history_list->dummy_tail->pre, msg);
    return;
  }
  DeleteList(users, free);

  if (modified_user->id[0] == '\0') {
    char *msg = malloc(sizeof(char) * (51 + id_len + 10));
    sprintf(msg, "[Error] [%s] User [uid = %d]'s id can't be blank",
            user.id, modified_user->uid);
    ReturnHistory(history_list->dummy_tail->pre, msg);
    return;
  }

  if (ErrorHandle(Update(modified_user, modified_user->uid, USER), 0)) return;
  if (modified_user->uid == user.uid) {
    memcpy(&user, modified_user, sizeof(User));
    id_len = strlen(user.id);
  }

  char *msg = malloc(sizeof(char) *
                     (32 + id_len + strlen(modified_user->id)));
  sprintf(msg, "[Info] [%s] Modify user [%s]'s info", user.id,
          modified_user->id);
  ReturnHistory(history_list->dummy_tail->pre, msg);
}

static void UserModify_TurnPage(bool direction) {
  UserModify *state = TopHistory()->state.user_modify;
  char *msg = MoveInList(&state->borrowrecords_start, state->borrowrecords, kUserModifyMax,
                         direction, "User", "User Modify");
  DrawUI(kUserModify, &user, state, msg);
}

static bool CmpGreaterBorrowRecordByReturnTime(const void * const lhs, const void * const rhs) {
  return strcmp(((BorrowRecord*)lhs)->returned_date, ((BorrowRecord*)rhs)->returned_date) >= 0;
}

static void inline UserSearchInfoDisplay(User *show_user, char *msg) {
  if (ErrorHandle(GetById(show_user, show_user->uid, USER), 0)) return;

  List *borrow_record = NewList();
  char *query = malloc(sizeof(char) * (10 + 10));
  sprintf(query, "user_uid=%d", show_user->uid);
  if (ErrorHandle(Filter(borrow_record, query, BORROWRECORD), 0)) {
    free(query);
    DeleteList(borrow_record, free);
    return;
  }
  free(query);
  SortList(borrow_record, CmpGreaterBorrowRecordByReturnTime);

  History *const new_history = malloc(sizeof(History));
  new_history->page = kUserModify;
  new_history->state.user_modify = malloc(sizeof(UserModify));
  new_history->state.user_modify->confirm_callback = UserModify_ConfirmCallback;
  new_history->state.user_modify->turn_page = UserModify_TurnPage;
  new_history->state.user_modify->user = malloc(sizeof(User));
  memcpy(new_history->state.user_modify->user, show_user, sizeof(User));
  new_history->state.user_modify->borrowrecords = borrow_record;
  new_history->state.user_modify->borrowrecords_start =
      borrow_record->dummy_head->nxt;
  PushBackHistory(new_history);

  if (!msg) {
    msg = malloc(sizeof(char) *
                 (30 + id_len + strlen(show_user->id)));
    sprintf(msg, "[Info] [%s] Show/Modify user [%s]", user.id,
            show_user->id);
  }
  Log(msg);
  DrawUI(kUserModify, &user, new_history->state.user_modify, msg);
}

static void UserSearch_InfoCallback(User *show_user) {
  UserSearchInfoDisplay(show_user, NULL);
}

static void inline UserSearchDisplay(char *keyword, char *msg) {
  if (user.whoami != ADMINISTRATOR) {
    char *msg = malloc(sizeof(char) * (49 + id_len));
    sprintf(msg, "[Error] [%s] Permission denied. Can't search users",
            user.id);
    ReturnHistory(history_list->dummy_tail->pre, msg);
    return;
  }

  List *results = NewList();
  if (ErrorHandle(Filter(results, keyword, USER), 0)) {
    DeleteList(results, free);
    return;
  }

  History *const new_history = malloc(sizeof(History));
  new_history->page = kUserSearch;
  new_history->state.user_search = malloc(sizeof(UserSearch));
  new_history->state.user_search->keyword = keyword;
  new_history->state.user_search->info_callback = UserSearch_InfoCallback;
  new_history->state.user_search->search_callback = UserSearch_SearchCallback;
  new_history->state.user_search->turn_page = UserSearch_TurnPage;
  new_history->state.user_search->user_result = results;
  new_history->state.user_search->user_result_start = results->dummy_head->nxt;
  PushBackHistory(new_history);

  if (!msg) {
    msg = malloc(sizeof(char) * (23 + id_len + strlen(keyword)));
    sprintf(msg, "[Info] [%s] User search %s", user.id, keyword);
  }
  Log(msg);
  DrawUI(kUserSearch, &user, new_history->state.user_search, msg);
}

static void UserSearch_SearchCallback(char *keyword) {
  char *new_keyword = malloc(sizeof(char) * (strlen(keyword) + 1));
  strcpy(new_keyword, keyword);
  UserSearchDisplay(new_keyword, NULL);
}

static void UserSearch_TurnPage(bool direction) {
  UserSearch *state = TopHistory()->state.user_search;
  char *msg = MoveInList(&state->user_result_start, state->user_result,
                         kUserSearchMax, direction, "Result", "User Search");
  DrawUI(kUserSearch, &user, state, msg);
}

static void LoginOrRegister_LoginCallback() {
  if (TopHistory()->page == kUserRegister) {
    if (TopHistory()->state.login_or_register->password[0] == '\0') {
      char *msg = malloc(sizeof(char) * 32);
      sprintf(msg, "[Error] password can't be blank");
      ReturnHistory(history_list->dummy_tail->pre, msg);
      return;
    }
    if (strcmp(TopHistory()->state.login_or_register->password,
               TopHistory()->state.login_or_register->repeat_password)) {
      char *msg = malloc(sizeof(char) * (47 + id_len));
      sprintf(msg, "[Error] Repeat password doesn't match password");
      ReturnHistory(history_list->dummy_tail->pre, msg);
      return;
    }

    User *new_user = TopHistory()->state.login_or_register->user;

    if (new_user->id[0] == '\0') {
      char *msg = malloc(sizeof(char) * 32);
      sprintf(msg, "[Error] id can't be blank");
      ReturnHistory(history_list->dummy_tail->pre, msg);
      return;
    }

    List *users = NewList();
    char *query = malloc(sizeof(char) * (10 + strlen(new_user->id)));
    sprintf(query, "id=%s", new_user->id);
    if (ErrorHandle(Filter(users, query, USER), 1, DB_ENTRY_EMPTY)) {
      free(query);
      DeleteList(users, free);
      return;
    }
    free(query);
    if (users->size != 0) {
      DeleteList(users, free);
      char *msg = malloc(sizeof(char) * 42);
      sprintf(msg, "[Error] Fail to register, id exists");
      ReturnHistory(history_list->dummy_tail->pre, msg);
      return;
    }
    DeleteList(users, free);

    if (ErrorHandle(GetNextPK(USER, &new_user->uid), 0)) return;
    RandStr(new_user->salt, 9);
    char pwd_type[59];
    sprintf(pwd_type, "%s%s", TopHistory()->state.login_or_register->password,
            new_user->salt);
    Sha256Sum(new_user->password, pwd_type, strlen(pwd_type));

    unsigned size_of_user_db;
    if (ErrorHandle(GetDBSize(USER, &size_of_user_db), 0)) return;

    char *msg = malloc(sizeof(char) * (45 + strlen(new_user->id)));

    if (!size_of_user_db) {  // the first user is admin
      new_user->whoami = ADMINISTRATOR;
      new_user->verified = TRUE;
      sprintf(msg, "[Info] [%s] Registered as an admin", new_user->id);
    } else {
      sprintf(msg, "[Info] [%s] Register. Wait for admin to verify",
              new_user->id);
      new_user->verified = FALSE;
    }
    if (ErrorHandle(Create(new_user, USER), 0)) {
      free(msg);
      return;
    }
    if (new_user->verified == TRUE) {
      memcpy(&user, new_user, sizeof(User));
      id_len = strlen(user.id);
    }

    History *const new_history = malloc(sizeof(History));
    new_history->page = kWelcome;
    PushBackHistory(new_history);

    Log(msg);
    DrawUI(kWelcome, &user, NULL, msg);
  } else {
    User *new_user = TopHistory()->state.login_or_register->user;

    List *users = NewList();
    char *query = malloc(sizeof(char) * (4 + strlen(new_user->id)));
    sprintf(query, "id=%s", new_user->id);
    if (ErrorHandle(Filter(users, query, USER), 1, DB_ENTRY_EMPTY)) {
      free(query);
      DeleteList(users, free);
      return;
    }
    free(query);
    if (users->size != 1) {
      DeleteList(users, free);
      char *msg = malloc(sizeof(char) * 31);
      sprintf(msg, "[Error] Can't find such a user");
      ReturnHistory(history_list->dummy_tail->pre, msg);
      return;
    }
    memcpy(new_user, users->dummy_head->nxt->value, sizeof(User));
    DeleteList(users, free);

    char pwd_type[59];
    sprintf(pwd_type, "%s%s", TopHistory()->state.login_or_register->password,
            new_user->salt);
    uint32_t sha_type[8];
    Sha256Sum(sha_type, pwd_type, strlen(pwd_type));
    if (memcmp(sha_type, new_user->password, sizeof(sha_type))) {
      char *msg = malloc(sizeof(char) * (45));
      sprintf(msg, "[Error] Password incorrect. Please try again");
      ReturnHistory(history_list->dummy_tail->pre, msg);
      return;
    }
    if (new_user->verified == FALSE) {
      char *msg = malloc(sizeof(char) * (60 + strlen(new_user->id)));
      sprintf(msg,
              "[Error] [%s] You haven't been verified. Please contact admin",
              new_user->id);
      ReturnHistory(history_list->dummy_tail->pre, msg);
      return;
    }
    memcpy(&user, new_user, sizeof(User));
    id_len = strlen(user.id);

    History *const new_history = malloc(sizeof(History));
    new_history->page = kWelcome;
    PushBackHistory(new_history);

    char *msg = malloc(sizeof(char) * (17 + id_len));
    sprintf(msg, "[Info] [%s] Log in", user.id);
    Log(msg);
    DrawUI(kWelcome, &user, NULL, msg);
  }
}

static void UserManagement_ApproveCallback(ListNode *user_node, bool approve) {
  User *new_user = user_node->value;
  char *msg =
      malloc(sizeof(char) * (27 + id_len + strlen(new_user->id)));
  if (approve) {
    new_user->verified = TRUE;
    if (ErrorHandle(Update(new_user, new_user->uid, USER), 0)) {
      free(msg);
      return;
    }
    sprintf(msg, "[Info] [%s] Approve user [%s]", user.id,
            new_user->id);
  } else {
    if (ErrorHandle(Delete(new_user->uid, USER), 0)) {
      free(msg);
      return;
    }
    sprintf(msg, "[Info] [%s] Disprove user [%s]", user.id,
            new_user->id);
  }
  ReturnHistory(history_list->dummy_tail->pre, msg);
}

static void UserManagement_DeleteCallback(ListNode *user_node) {
  User *new_user = user_node->value;
  if (new_user->whoami == ADMINISTRATOR) {
    char *msg = malloc(sizeof(char) * (38 + id_len));
    sprintf(msg, "[Error] [%s] Can't delete admin account",
            user.id);
    ReturnHistory(history_list->dummy_tail->pre, msg);
    return;
  }
  if (ErrorHandle(Delete(new_user->uid, USER), 0)) return;
  char *msg =
      malloc(sizeof(char) * (25 + id_len + strlen(new_user->id)));
  sprintf(msg, "[Info] [%s] Delete user [%s]", user.id,
          new_user->id);
  ReturnHistory(history_list->dummy_tail->pre, msg);
}

static void UserManagement_TurnPage(bool direction, bool type) {
  UserManagement *state = TopHistory()->state.user_management;
  char *msg;
  if (type)
    msg = MoveInList(&state->users_start, state->users, kUserManagementUsersMax,
                     direction, "Verified User", "User Management");
  else
    msg = MoveInList(&state->to_be_verified_start, state->to_be_verified,
                     kUserManagementToBeVerifiedMax, direction,
                     "To Be Verified User", "User Management");
  DrawUI(kUserManagement, &user, state, msg);
}

static void BookDisplayAdminDisplay(char *msg) {
  if (user.whoami != ADMINISTRATOR) {
    char *msg = malloc(sizeof(char) * (60 + id_len));
    sprintf(msg,
            "[Error] [%s] Permission denied. Can't open Page BorrowDisplay",
            user.id);
    ReturnHistory(history_list->dummy_tail->pre, msg);
    return;
  }

  if (ErrorHandle(GetById(TopHistory()->state.book_display->book,
                          TopHistory()->state.book_display->book->uid, BOOK),
                  0))
    return;

  List *borrow_record = NewList();
  char *query = malloc(sizeof(char) * 20);
  sprintf(query, "book_uid=%d", TopHistory()->state.book_display->book->uid);
  if (ErrorHandle(Filter(borrow_record, query, BORROWRECORD), 0)) {
    free(query);
    DeleteList(borrow_record, free);
    return;
  }
  free(query);
  SortList(borrow_record, CmpGreaterBorrowRecordByReturnTime);

  History *const new_history = malloc(sizeof(History));
  new_history->page = kBorrowDisplay;
  new_history->state.borrow_display = malloc(sizeof(BorrowDisplay));
  new_history->state.borrow_display->book_id =
      malloc(sizeof(TopHistory()->state.book_display->book->id));
  strcpy(new_history->state.borrow_display->book_id,
         TopHistory()->state.book_display->book->id);
  new_history->state.borrow_display->turn_page = BorrowDisplay_TurnPage;
  new_history->state.borrow_display->borrow_record = borrow_record;
  new_history->state.borrow_display->borrow_record_start =
      borrow_record->dummy_head->nxt;
  PushBackHistory(new_history);

  if (!msg) {
    msg = malloc(sizeof(char) *
                 (46 + id_len +
                  strlen(new_history->state.borrow_display->book_id)));
    sprintf(msg, "[Info] [%s] Open Page BorrowDisplay for book [%s]",
            user.id, new_history->state.borrow_display->book_id);
  }
  Log(msg);
  DrawUI(kBorrowDisplay, &user, new_history->state.borrow_display, msg);
}

static void BookDisplay_AdminCallback() { BookDisplayAdminDisplay(NULL); }

static void BookDisplay_CoverCallback() {
  char image_path[MAX_PATH + 1];
  try {
    SelectFile("JPG image (*.jpg|*.jpeg|*.jpe)\0*.jpg;*.jpeg;*.jpe\0", "jpg",
               FALSE, image_path, MAX_PATH);
    except(ErrorException) {
      char *msg = malloc(sizeof(char) * (34 + id_len));
      sprintf(msg, "[Error] [%s] Fail to open the image", user.id);
      DrawUI(kBookModify, &user, TopHistory()->state.book_display, msg);
      return;
    }
  }
  endtry;
  const int uid = TopHistory()->state.book_display->book->uid;
  char *command =
      malloc(sizeof(char) * (25 + sizeof(image_path) + lib_path_len + 10));
  sprintf(command, "copy /Y \"%s\" \"%s\\image\\%d.jpg\"", image_path, lib_path,
          uid);

  char *msg = malloc(sizeof(char) * (51 + id_len + 10));
  if (system(command)) {
    sprintf(msg, "[Error] [%s] Fail to change the book(uid = %d)'s cover",
            user.id, uid);
    free(command);
    DrawUI(TopHistory()->page, &user, TopHistory()->state.book_display, msg);
    return;
  }
  free(command);
  loadImage(image_path, &TopHistory()->state.book_display->book_cover);

  sprintf(msg, "[Info] [%s] Change the book(uid = %d)'s cover", user.id,
          uid);
  Log(msg);
  DrawUI(TopHistory()->page, &user, TopHistory()->state.book_display, msg);
}

static void BookDisplay_ConfirmCallback() {
  if (user.whoami != ADMINISTRATOR) {
    char *msg = malloc(sizeof(char) * (60 + id_len));
    if (TopHistory()->page == kBookModify)
      sprintf(msg, "[Error] [%s] Permission denied. Can't modify any book",
              user.id);
    else
      sprintf(msg, "[Error] [%s] Permission denied. Can't init any book",
              user.id);
    ReturnHistory(history_list->dummy_tail->pre, msg);
    return;
  }

  Book *new_book = TopHistory()->state.book_display->book;
  if (new_book->id[0] == '\0') {
    char *msg = malloc(sizeof(char) * (36 + id_len));
    sprintf(msg, "[Error] [%s] Book's id can't be blank", user.id);
    ReturnHistory(history_list->dummy_tail->pre, msg);
    return;
  }

  List *books = NewList();
  char *query = malloc(sizeof(char) * (10 + strlen(new_book->id)));
  sprintf(query, "id=%s", new_book->id);
  free(query);
  if (books->size > 1 ||
      (books->size == 1 &&
       ((Book *)books->dummy_head->nxt->value)->uid != new_book->uid)) {
    DeleteList(books, free);
    char *msg = malloc(sizeof(char) * 42);
    sprintf(msg, "[Error] Fail to modify, id exists");
    ReturnHistory(history_list->dummy_tail->pre, msg);
    return;
  }
  DeleteList(books, free);

  if (TopHistory()->page == kBookInit) {
    if (ErrorHandle(Create(new_book, BOOK), 0)) return;

    char *msg =
        malloc(sizeof(char) * (25 + id_len + strlen(new_book->id)));
    sprintf(msg, "[Info] [%s] Init book [%s]", user.id, new_book->id);
    ReturnHistory(history_list->dummy_tail->pre, msg);
  } else {
    if (ErrorHandle(Update(new_book, new_book->uid, BOOK), 0)) return;

    char *msg =
        malloc(sizeof(char) * (25 + id_len + strlen(new_book->id)));
    sprintf(msg, "[Info] [%s] Modify book [%s]", user.id,
            new_book->id);
    ReturnHistory(history_list->dummy_tail->pre, msg);
  }
}

static void BookDisplay_DeleteCallback() {
  if (user.whoami != ADMINISTRATOR) {
    char *msg = malloc(sizeof(char) * (53 + id_len));
    sprintf(msg, "[Error] [%s] Permission denied. Can't delete the book",
            user.id);
    ReturnHistory(history_list->dummy_tail->pre, msg);
    return;
  }
  Book *new_book = TopHistory()->state.book_display->book;
  if (TopHistory()->page == kBookModify) {
    if (ErrorHandle(Delete(new_book->uid, BOOK), 0)) return;
  }

  char *command = malloc(sizeof(char) * (lib_path_len + 19 + 10));
  sprintf(command, "del /F %s\\image\\%d.jpg", lib_path, new_book->uid);
  system(command);
  free(command);

  char *msg =
      malloc(sizeof(char) * (25 + id_len + strlen(new_book->id)));
  sprintf(msg, "[Info] [%s] Delete book [%s]", user.id, new_book->id);
  ReturnHistory(history_list->dummy_tail->pre->pre, msg);
}

static void BookDisplay_BorrowCallback() {
  BookSearch_BorrowCallback(TopHistory()->state.book_display->book);
}

static void BookDisplay_CopyPasteCallback() {
  Book *book = TopHistory()->state.book_display->book;
  const unsigned old_uid = book->uid;
  if (ErrorHandle(GetById(book, old_uid, BOOK), 0) ||
      ErrorHandle(GetNextPK(BOOK, &book->uid), 0)) {
    book->uid = old_uid;
    char *msg = malloc(sizeof(char) * (34 + id_len));
    sprintf(msg, "[Error] [%s] Fail to copy and paste", user.id);
    ReturnHistory(history_list->dummy_tail->pre, msg);
  }

  book->number_on_the_shelf = 0;
  if (ErrorHandle(Create(book, BOOK), 0)) {
    book->uid = old_uid;
    char *msg = malloc(sizeof(char) * (34 + id_len));
    sprintf(msg, "[Error] [%s] Fail to copy and paste", user.id);
    ReturnHistory(history_list->dummy_tail->pre, msg);  
  }

  const size_t image_path_len = 7 + lib_path_len;
  char *image_path = malloc(sizeof(char) * (image_path_len + 1));
  sprintf(image_path, "%s\\image\\", lib_path);
  char *command = malloc(sizeof(char) * (24 + image_path_len * 2 + 20));
  sprintf(command, "copy /Y \"%s%d.jpg\" \"%s%d.jpg\"", image_path, old_uid,
          image_path, book->uid);
  system(command);

  char *msg = malloc(sizeof(char) * (63 + id_len + strlen(book->id)));
  sprintf(msg,
          "[Info] [%s] Copy and paste book [%s], set number on the shelf to 0",
          user.id, book->id);
  Navigation_BookDisplayOrInit(book, 0, msg);
}

static void BorrowDisplay_TurnPage(bool direction) {
  BorrowDisplay *state = TopHistory()->state.borrow_display;
  char *msg =
      MoveInList(&state->borrow_record_start, state->borrow_record,
                 kBorrowDisplayMax, direction, "Record", "Borrow Display");
  DrawUI(kBorrowDisplay, &user, state, msg);
}

static void Library_BookCallback(ListNode *book) {
  Navigation_BookDisplayOrInit(book->value, 0, NULL);
}

static bool CmpById(const void *const lhs, const void *const rhs) {
  return strcmp(((Book *)lhs)->id, ((Book *)rhs)->id) <= 0;
}

static bool CmpByTitle(const void *const lhs, const void *const rhs) {
  return strcmp(((Book *)lhs)->title, ((Book *)rhs)->title) <= 0;
}

static bool CmpByAuthor(const void *const lhs, const void *const rhs) {
  return strcmp(((Book *)lhs)->authors[0], ((Book *)rhs)->authors[0]) <= 0;
}

static void Library_SortCallback(SortKeyword sort_keyword) {
  // 由于图片模式不可排序，所以不需要对 book_covers 做处理
  char *msg = malloc(sizeof(char) * (33 + id_len));
  switch (sort_keyword) {
    case kId:
      SortList(TopHistory()->state.library->books, CmpById);
      sprintf(msg, "[Info] [%s] sort books by id", user.id);
      break;
    case kTitle:
      SortList(TopHistory()->state.library->books, CmpByTitle);
      sprintf(msg, "[Info] [%s] sort books by title", user.id);
      break;
    case kAuthor:
      SortList(TopHistory()->state.library->books, CmpByAuthor);
      sprintf(msg, "[Info] [%s] sort books by author", user.id);
      break;
    default:
      Log("[Debug] Unknown sort_keyword in Library_SortCallback");
      Error("Unknown nav_page");
      break;
  }
  TopHistory()->state.library->books_start = TopHistory()->state.library->books->dummy_head->nxt;
  Log(msg);
  DrawUI(kLibrary, &user, TopHistory()->state.library, msg);
}

static void Library_SwitchCallback() {
  if (TopHistory()->state.library->type == kList) {
    Navigation_Library(NULL);
  } else {
    List *books = NewList();
    if (ErrorHandle(Filter(books, "", BOOK), 0)) {
      DeleteList(books, free);
      return;
    }

    History *const new_history = malloc(sizeof(History));
    new_history->page = kLibrary;
    new_history->state.library = malloc(sizeof(Library));
    new_history->state.library->type = kList;
    new_history->state.library->sort_callback = Library_SortCallback;
    new_history->state.library->book_callback = Library_BookCallback;
    new_history->state.library->switch_callback = Library_SwitchCallback;
    new_history->state.library->turn_page = Library_TurnPage;
    new_history->state.library->books = books;
    new_history->state.library->books_start = books->dummy_head->nxt;
    new_history->state.library->book_covers = NULL;
    new_history->state.library->books_covers_start = NULL;
    PushBackHistory(new_history);

    char *msg = malloc(sizeof(char) * (41 + id_len));
    sprintf(msg, "[Info] [%s] Open Page Library (list mode)", user.id);
    Log(msg);
    DrawUI(kLibrary, &user, new_history->state.library, msg);
  }
}

void Library_TurnPage(bool direction) {
  Library *state = TopHistory()->state.library;
  MoveInList(&state->books_covers_start, state->book_covers, kLibraryMax,
             direction, "", "");
  char *msg = MoveInList(&state->books_start, state->books, kLibraryMax,
                         direction, "Book", "Library");
  DrawUI(kLibrary, &user, state, msg);
}

static void *const StrCpy(void *const str) {
  char *ret = malloc(sizeof(char) * (strlen(str) + 1));
  strcpy(ret, str);
  return ret;
}

static void Statistics_SelectCallback(ListNode *catalog) {
  List *borrow_records = NewList();
  Book *book = malloc(sizeof(Book));
  if (ErrorHandle(Filter(borrow_records, "", BORROWRECORD), 0)) {
    DeleteList(borrow_records, free);
    return;
  }
  if (strcmp(catalog->value, "ALL")) {
    for (const ListNode *cur_node = borrow_records->dummy_head->nxt;
         cur_node != borrow_records->dummy_tail;) {
      if (ErrorHandle(
              GetById(book, ((BorrowRecord *)cur_node->value)->book_uid, BOOK),
              0)) {
        DeleteList(borrow_records, free);
        free(book);
        return;
      }
      if (strcmp(book->category, catalog->value))
        cur_node = EraseList(borrow_records, cur_node, NULL);
      else
        cur_node = cur_node->nxt;
    }
  }

  History *const new_history = malloc(sizeof(History));
  new_history->page = kStatistics;
  new_history->state.statistics = malloc(sizeof(Statistics));
  new_history->state.statistics->select_callback = Statistics_SelectCallback;
  new_history->state.statistics->turn_page = Statistics_TurnPage;
  new_history->state.statistics->catalogs =
      DuplicateList(TopHistory()->state.statistics->catalogs, StrCpy);
  new_history->state.statistics->catalogs_start =
      new_history->state.statistics->catalogs->dummy_head->nxt;
  new_history->state.statistics->borrow_record = borrow_records;
  new_history->state.statistics->borrow_record_start =
      borrow_records->dummy_head->nxt;
  PushBackHistory(new_history);

  char *msg =
      malloc(sizeof(char) * (46 + id_len + strlen(catalog->value)));
  sprintf(msg, "[Info] [%s] Open Page Statistics with category %s",
          user.id, (char *)catalog->value);
  Log(msg);
  DrawUI(kStatistics, &user, new_history->state.statistics, msg);
}

static void Statistics_TurnPage(bool direction, bool type) {
  Statistics *state = TopHistory()->state.statistics;
  char *msg;
  if (type)
    msg = MoveInList(&state->borrow_record_start, state->borrow_record,
                     kStatisticsBorrowRecordMax, direction, "Record",
                     "Statistics");
  else
    msg =
        MoveInList(&state->catalogs_start, state->catalogs,
                   kStatisticsCatalogsMax, direction, "Category", "Statistics");
  DrawUI(kStatistics, &user, state, msg);
}

static bool CmpLessBorrowRecordByReturnTime(const void *const lhs,
                                               const void *const rhs) {
  return strcmp(((BorrowRecord *)lhs)->returned_date,
                ((BorrowRecord *)rhs)->returned_date) <= 0;
}

static inline void Navigation_LendAndBorrow(char *msg) {
  if (InitCheck(FALSE)) return;
  List *borrow_records_list = NewList();
  char *query = malloc(sizeof(char) * (31 + 10));
  sprintf(query, "user_uid=%d&book_status=1", user.uid);
  if (ErrorHandle(Filter(borrow_records_list, query, BORROWRECORD), 0)) {
    DeleteList(borrow_records_list, free);
    free(query);
    return;
  }
  free(query);
  SortList(borrow_records_list, CmpLessBorrowRecordByReturnTime);

  List *books = NewList();
  for (ListNode *cur_node = borrow_records_list->dummy_head->nxt;
       cur_node != borrow_records_list->dummy_tail; cur_node = cur_node->nxt) {
    Book *book = malloc(sizeof(Book));
    if (ErrorHandle(GetById(book, ((BorrowRecord *)cur_node->value)->book_uid,
                            BOOK),
                    0)) {
      DeleteList(borrow_records_list, free);
      DeleteList(books, free);
      return;
    }
    InsertList(books, books->dummy_tail, book);
  }

  History *const new_history = malloc(sizeof(History));
  new_history->page = kLendAndBorrow;
  new_history->state.lend_and_borrow = malloc(sizeof(LendAndBorrow));
  new_history->state.lend_and_borrow->return_callback =
      LendAndBorrow_ReturnCallback;
  new_history->state.lend_and_borrow->search_callback =
      LendAndBorrow_SearchCallback;
  new_history->state.lend_and_borrow->turn_page = LendAndBorrow_TurnPage;
  new_history->state.lend_and_borrow->borrow_records = borrow_records_list;
  new_history->state.lend_and_borrow->borrow_records_start =
      borrow_records_list->dummy_head->nxt;
  new_history->state.lend_and_borrow->books = books;
  new_history->state.lend_and_borrow->books_start = books->dummy_head->nxt;
  PushBackHistory(new_history);

  if (!msg) {
    msg = malloc(sizeof(char) * (34 + id_len));
    sprintf(msg, "[Info] [%s] Open Page LendAndBorrow", user.id);
  }
  Log(msg);
  DrawUI(kLendAndBorrow, &user, new_history->state.lend_and_borrow, msg);
}

static inline void Navigation_BookSearch(char *msg) {
  if (InitCheck(FALSE)) return;
  char *keyword = malloc(sizeof(char));
  *keyword = '\0';
  BookSearchDisplay(keyword, msg);
}

static inline void Navigation_UserSearch(char *msg) {
  if (InitCheck(FALSE)) return;
  char *keyword = malloc(sizeof(char));
  *keyword = '\0';
  UserSearchDisplay(keyword, msg);
}

// type = 0 => Manual
static inline void Navigation_ManualOrAbout(bool type, char *msg) {
  History *const new_history = malloc(sizeof(History));
  new_history->state.manual_and_about = malloc(sizeof(ManualAndAbout));

  if (type)
    new_history->page = kAbout;
  else
    new_history->page = kManual;
  PushBackHistory(new_history);

  if (!msg) {
    msg = malloc(sizeof(char) * (27 + id_len));
    if (type)
      sprintf(msg, "[Info] [%s] Open Page About", user.id);
    else
      sprintf(msg, "[Info] [%s] Open Page Manual", user.id);
  }
  Log(msg);
  DrawUI(kAbout, &user, new_history->state.manual_and_about, msg);
}

// type = 0 => LogIn
static inline void Navigation_UserLogInOrRegister(bool type, char *msg) {
  if (InitCheck(TRUE)) return;
  memset(&user, 0x00, sizeof(User));
  id_len = 0;

  ClearHistory();
  History *const new_history = malloc(sizeof(History));
  if (type)
    new_history->page = kUserRegister;
  else
    new_history->page = kUserLogIn;
  new_history->state.login_or_register = malloc(sizeof(LoginOrRegister));
  new_history->state.login_or_register->user = malloc(sizeof(User));
  memset(new_history->state.login_or_register->user, 0x00, sizeof(User));
  new_history->state.login_or_register->login_callback =
      LoginOrRegister_LoginCallback;
  PushBackHistory(new_history);

  if (!msg) {
    msg = malloc(sizeof(char) * 38);
    sprintf(msg, "[Info] Clear history, try to %s",
            type ? "register" : "log in");
  }
  Log(msg);
  DrawUI(new_history->page, &user, new_history->state.login_or_register, msg);
}

static inline void Navigation_UserLogOut(char *msg) {
  if (InitCheck(FALSE)) return;
  ClearHistory();
  History *const new_history = malloc(sizeof(History));
  new_history->page = kWelcome;
  PushBackHistory(new_history);

  if (!msg) {
    msg = malloc(sizeof(char) * (36 + id_len));
    sprintf(msg, "[Info] [%s] Clear history and log out", user.id);
  }

  memset(&user, 0x00, sizeof(User));
  id_len = 0;

  Log(msg);
  DrawUI(kWelcome, &user, new_history->state.login_or_register, msg);
}

static inline void Navigation_UserModify(char *msg) {
  if (InitCheck(FALSE)) return;
  UserSearchInfoDisplay(&user, msg);
}

static inline void Navigation_UserManagement(char *msg) {
  if (InitCheck(FALSE)) return;
  if (user.whoami != ADMINISTRATOR) {
    char *msg = malloc(sizeof(char) * (49 + id_len));
    sprintf(msg, "[Error] [%s] Permission denied. Can't manage users",
            user.id);
    ReturnHistory(history_list->dummy_tail->pre, msg);
    return;
  }

  List *to_be_verified = NewList();
  if (ErrorHandle(Filter(to_be_verified, "verified=0", USER), 0)) {
    DeleteList(to_be_verified, free);
    return;
  }

  List *verified = NewList();
  if (ErrorHandle(Filter(verified, "verified=1", USER), 0)) {
    DeleteList(verified, free);
    return;
  }

  History *const new_history = malloc(sizeof(History));
  new_history->page = kUserManagement;
  new_history->state.user_management = malloc(sizeof(UserManagement));
  new_history->state.user_management->approve_callback =
      UserManagement_ApproveCallback;
  new_history->state.user_management->delete_callback =
      UserManagement_DeleteCallback;
  new_history->state.user_management->turn_page = UserManagement_TurnPage;
  new_history->state.user_management->to_be_verified = to_be_verified;
  new_history->state.user_management->to_be_verified_start =
      to_be_verified->dummy_head->nxt;
  new_history->state.user_management->users = verified;
  new_history->state.user_management->users_start = verified->dummy_head->nxt;
  PushBackHistory(new_history);

  if (!msg) {
    msg = malloc(sizeof(char) * (36 + id_len));
    sprintf(msg, "[Info] [%s] Open Page User Management", user.id);
  }
  Log(msg);
  DrawUI(kUserManagement, &user, new_history->state.user_management, msg);
}

static inline void Navigation_Library(char *msg) {
  if (InitCheck(FALSE)) return;
  List *books = NewList();
  if (ErrorHandle(Filter(books, "", BOOK), 0)) {
    DeleteList(books, free);
    return;
  }

  List *book_covers = NewList();
  const size_t image_path_len = 8 + lib_path_len;
  char *image_path = malloc(sizeof(char) * (image_path_len + 14));
  sprintf(image_path, "%s\\image\\", lib_path);
  for (ListNode *cur_node = books->dummy_head->nxt;
       cur_node != books->dummy_tail; cur_node = cur_node->nxt) {
    LibImage *image = malloc(sizeof(LibImage));
    sprintf(image_path + image_path_len - 1, "%d.jpg",
            ((Book *)cur_node->value)->uid);
    if (!_access(image_path, 4))
      loadImage(image_path, image);    
    else
      *image = unknown_cover;
    InsertList(book_covers, book_covers->dummy_tail, image);
  }

  History *const new_history = malloc(sizeof(History));
  new_history->page = kLibrary;
  new_history->state.library = malloc(sizeof(Library));
  new_history->state.library->type = kPicture;
  new_history->state.library->sort_callback = Library_SortCallback;
  new_history->state.library->book_callback = Library_BookCallback;
  new_history->state.library->switch_callback = Library_SwitchCallback;
  new_history->state.library->turn_page = Library_TurnPage;
  new_history->state.library->books = books;
  new_history->state.library->books_start = books->dummy_head->nxt;
  new_history->state.library->book_covers = book_covers;
  new_history->state.library->books_covers_start = book_covers->dummy_head->nxt;
  PushBackHistory(new_history);

  if (!msg) {
    msg = malloc(sizeof(char) * (41 + id_len));
    sprintf(msg, "[Info] [%s] Open Page Library (image mode)", user.id);
  }
  Log(msg);
  DrawUI(kLibrary, &user, new_history->state.library, msg);
}

// type == 0 => Open
static inline void Navigation_OpenOrInitLibrary(bool type, char *msg) {
  try {
    SelectFolder("请选择保存图书库的文件夹", lib_path);
    except(ErrorException) {
      char *msg = malloc(sizeof(char) * (56 + id_len));
      if (type)
        sprintf(msg,
                "[Error] [%s] Fail to init the library. Path doesn't exist",
                user.id);
      else
        sprintf(msg,
                "[Error] [%s] Fail to open the library. Path doesn't exist",
                user.id);
      ReturnHistory(history_list->dummy_tail->pre, msg);
      return;
    }
  }
  endtry;
  lib_path_len = strlen(lib_path);

  // copy *.swp.db to *.db and remove *.swp.db
  Navigation_SaveLibrary(0, NULL);
  char *command = malloc(sizeof(char) * (8 + strlen(borrowrecord_db_dir)));

  if (ErrorHandle(CloseDBConnection(USER), 1, DB_NOT_OPEN)) {
    free(command);
    return;
  }
  sprintf(command, "del /F %s", user_db_dir);
  system(command);

  db_open = FALSE;

  if (ErrorHandle(CloseDBConnection(BOOK), 1, DB_NOT_OPEN)) {
    free(command);
    return;
  }
  sprintf(command, "del /F %s", book_db_dir);
  system(command);

  if (ErrorHandle(CloseDBConnection(BORROWRECORD), 1, DB_NOT_OPEN)) {
    free(command);
    return;
  }
  sprintf(command, "del /F %s", borrowrecord_db_dir);
  system(command);
  free(command);

  if (type) {
    char *command = malloc(sizeof(char) * (15 + lib_path_len));
    sprintf(command, "mkdir \"%s\\image\"", lib_path);
    if (system(command)) {
      free(command);
      char *msg = malloc(sizeof(char) * (63 + id_len));
      sprintf(
          msg,
          "[Error] [%s] Fail to init the library. Can't create image folder",
          user.id);
      ReturnHistory(history_list->dummy_tail->pre, msg);
      return;
    }
    free(command);
  }

  int flag = 0;  // 0 => 无事发生 1=> 有swap文件 2=> 无文件

  sprintf(user_db_dir, "%s%s", lib_path, "\\user.swp.db");
  if (!_access(user_db_dir, 6)) {  // swap file exists
    flag |= 1;
  } else {
    char *user_database_path = malloc(sizeof(char) * (lib_path_len + 9));
    sprintf(user_database_path, "%s\\user.db", lib_path);
    if (!_access(user_database_path, 6)) {  // user database exists
      char *command =
          malloc(sizeof(char) * (14 + lib_path_len + 8 + lib_path_len + 12));
      sprintf(command, "copy /Y \"%s\" \"%s\"", user_database_path,
              user_db_dir);
      system(command);
      free(command);
    } else {  // book database doesn't exist
      char *command =
          malloc(sizeof(char) * (15 + lib_path_len + 12));
      sprintf(command, "copy /Y nul \"%s\"", user_db_dir);
      system(command);
      free(command);
      flag |= 2;
    }
    free(user_database_path);
  }
  if (ErrorHandle(OpenDBConnection(user_db_dir, USER), 2, DB_NOT_OPEN,
                  DB_ENTRY_EMPTY))
    return;

  sprintf(book_db_dir, "%s%s", lib_path, "\\book.swp.db");
  if (!_access(book_db_dir, 6)) {  // swap file exists
    flag |= 1;
  } else {
    char *book_database_path = malloc(sizeof(char) * (lib_path_len + 9));
    sprintf(book_database_path, "%s\\book.db", lib_path);
    if (!_access(book_database_path, 6)) {  // book database exists
      char *command =
          malloc(sizeof(char) * (14 + lib_path_len + 8 + lib_path_len + 12));
      sprintf(command, "copy /Y \"%s\" \"%s\"", book_database_path,
              book_db_dir);
      system(command);
      free(command);
    } else {  // book database doesn't exist
      char *command = malloc(sizeof(char) * (15 + lib_path_len + 12));
      sprintf(command, "copy /Y nul \"%s\"", book_db_dir);
      system(command);
      free(command);
      flag |= 2;
    }
    free(book_database_path);
  }
  if (ErrorHandle(OpenDBConnection(book_db_dir, BOOK), 2, DB_NOT_OPEN,
                  DB_ENTRY_EMPTY)) {
    CloseDBConnection(USER);
    return;
  }

  sprintf(borrowrecord_db_dir, "%s%s", lib_path, "\\borrowrecord.swp.db");
  if (!_access(borrowrecord_db_dir, 6)) {  // swap file exists
    flag |= 1;
  } else {
    char *borrowrecord_database_path =
        malloc(sizeof(char) * (lib_path_len + 17));
    sprintf(borrowrecord_database_path, "%s\\borrowrecord.db", lib_path);
    if (!_access(borrowrecord_database_path,
                 6)) {  // borrowrecord database exists
      char *command =
          malloc(sizeof(char) * (14 + lib_path_len + 16 + lib_path_len + 20));
      sprintf(command, "copy /Y \"%s\" \"%s\"", borrowrecord_database_path,
              borrowrecord_db_dir);
      system(command);
      free(command);
    } else {  // borrowrecord database doesn't exist
      char *command = malloc(sizeof(char) * (15 + lib_path_len + 20));
      sprintf(command, "copy /Y nul \"%s\"", borrowrecord_db_dir);
      system(command);
      free(command);
      flag |= 2;
    }
    free(borrowrecord_database_path);
  }
  if (ErrorHandle(OpenDBConnection(borrowrecord_db_dir, BORROWRECORD), 2,
                  DB_NOT_OPEN, DB_ENTRY_EMPTY)) {
    CloseDBConnection(USER);
    CloseDBConnection(BOOK);
    return;
  }

  ClearHistory();
  History *const new_history = malloc(sizeof(History));
  new_history->page = kWelcome;
  PushBackHistory(new_history);

  if (!msg) {
    msg = malloc(sizeof(char) * (114 + lib_path_len + id_len));
    if (type) {
      if (flag != 2)
        sprintf(msg,
                "[Error] [%s] Log out, Clear history and init library from %s, "
                "where "
                "already exists an eLibrary",
                user.id, lib_path);
      else
        sprintf(msg,
                "[Info] [%s] Log out, Clear history and init library from %s",
                user.id, lib_path);
    } else {
      if ((flag & 2) == 2) {
        sprintf(msg,
                "[Error] [%s] Log out, Clear history and fail to open library "
                "from %s, "
                "init one at there, undefined behavior may occur",
                user.id, lib_path);
        char *command = malloc(sizeof(char) * (15 + lib_path_len));
        sprintf(command, "mkdir \"%s\\image\"", lib_path);
        system(command);
        free(command);
      } else if ((flag & 1) == 1) {
        sprintf(msg,
                "[Warning] [%s] Log out, Clear history and open library from "
                "%s using "
                "swap file",
                user.id, lib_path);
      } else if (!flag) {
        sprintf(msg,
                "[Info] [%s] Log out, Clear history and open library from %s",
                user.id, lib_path);
      }
    }
  }

  db_open = TRUE;
  memset(&user, 0x00, sizeof(User));
  id_len = 0;

  Log(msg);
  DrawUI(kWelcome, &user, NULL, msg);
}

// type = 0 => 不回退到上一个界面
static inline void Navigation_SaveLibrary(bool type, char *msg) {
  if (InitCheck(TRUE)) return;
  char *command =
      malloc(sizeof(char) * (14 + lib_path_len + 16 + lib_path_len + 20));

  // TODO:(TO/GA) delete them
  if (ErrorHandle(CloseDBConnection(USER), 0)) return;
  if (ErrorHandle(CloseDBConnection(BOOK), 0)) return;
  if (ErrorHandle(CloseDBConnection(BORROWRECORD), 0)) return;
  db_open = FALSE;

  char *user_database_path = malloc(sizeof(char) * (lib_path_len + 9));
  sprintf(user_database_path, "%s\\user.db", lib_path);
  sprintf(command, "copy /Y \"%s\" \"%s\"", user_db_dir, user_database_path);
  free(user_database_path);
  system(command);

  char *book_database_path = malloc(sizeof(char) * (lib_path_len + 9));
  sprintf(book_database_path, "%s\\book.db", lib_path);
  sprintf(command, "copy /Y \"%s\" \"%s\"", book_db_dir, book_database_path);
  free(book_database_path);
  system(command);

  char *borrowrecord_database_path = malloc(sizeof(char) * (lib_path_len + 17));
  sprintf(borrowrecord_database_path, "%s\\borrowrecord.db", lib_path);
  sprintf(command, "copy /Y \"%s\" \"%s\"", borrowrecord_db_dir,
          borrowrecord_database_path);
  free(borrowrecord_database_path);
  system(command);

  free(command);

  // TODO:(TO/GA) delete them
  if (ErrorHandle(OpenDBConnection(user_db_dir, USER), 0)) return;
  if (ErrorHandle(OpenDBConnection(book_db_dir, BOOK), 0)) return;
  if (ErrorHandle(OpenDBConnection(borrowrecord_db_dir, BORROWRECORD), 0)) return;
  db_open = TRUE;

  if (type) {
    if (!msg) {
      msg = malloc(sizeof(char) * (23 + id_len));
      sprintf(msg, "[Info] [%s] Save library", user.id);
    }
    ReturnHistory(history_list->dummy_tail->pre, msg);
  }
}

// type = 0 => Display
static void Navigation_BookDisplayOrInit(Book *book, bool type, char *msg) {
  Book *new_book;
  if (type) {
    new_book = book;
  } else {
    new_book = malloc(sizeof(Book));
    if (ErrorHandle(GetById(new_book, book->uid, BOOK), 0)) {
      free(new_book);
      return;
    }
  }

  History *const new_history = malloc(sizeof(History));
  if (type) {
    new_history->page = kBookInit;
  } else {
    if (user.whoami == ADMINISTRATOR)
      new_history->page = kBookModify;
    else
      new_history->page = kBookDisplay;
  }
  new_history->state.book_display = malloc(sizeof(BookDisplay));
  new_history->state.book_display->admin_callback = BookDisplay_AdminCallback;
  new_history->state.book_display->cover_callback = BookDisplay_CoverCallback;
  new_history->state.book_display->confirm_callback =
      BookDisplay_ConfirmCallback;
  new_history->state.book_display->delete_callback = BookDisplay_DeleteCallback;
  new_history->state.book_display->borrow_callback = BookDisplay_BorrowCallback;
  new_history->state.book_display->copy_paste_callback = BookDisplay_CopyPasteCallback;
  new_history->state.book_display->book = new_book;
  if (!type) {
    char *image_path = malloc(sizeof(char) * (12 + lib_path_len + 10));
    sprintf(image_path, "%s\\image\\%d.jpg", lib_path, book->uid);
    if (!_access(image_path, 4))
      loadImage(image_path, &new_history->state.book_display->book_cover);
    else
      new_history->state.book_display->book_cover = unknown_cover;
  } else {
    new_history->state.book_display->book_cover = edit_cover;
  }
  PushBackHistory(new_history);

  if (!msg) {
    if (type) {
      msg = malloc(sizeof(char) * (30 + id_len));
      sprintf(msg, "[Info] [%s] Open book init page", user.id);
    } else {
      msg = malloc(sizeof(char) * (36 + id_len + strlen(new_book->id)));
      if (user.whoami == ADMINISTRATOR)
        sprintf(msg, "[Info] [%s] Open book modify page [%s]", user.id, new_book->id);
      else
        sprintf(msg, "[Info] [%s] Open book display page [%s]", user.id, new_book->id);
    }
  }
  Log(msg);
  DrawUI(new_history->page, &user, new_history->state.book_display, msg);
}

static inline void Navigation_BookInit(char *msg) {
  if (InitCheck(FALSE)) return;
  Book *book = malloc(sizeof(Book));
  memset(book, 0, sizeof(Book));
  if (ErrorHandle(GetNextPK(BOOK, &book->uid), 0)) {
    free(book);
    return;
  }
  Navigation_BookDisplayOrInit(book, 1, msg);
}

static bool StrLess(const void *const lhs, const void *rhs) {
  return strcmp(lhs, rhs) <= 0;
}
static bool StrSame(const void *const lhs, const void *rhs) {
  return strcmp(lhs, rhs) == 0;
}

static inline void Navigation_Statistics(char *msg) {
  if (InitCheck(FALSE)) return;
  if (user.whoami != ADMINISTRATOR) {
    char *msg = malloc(sizeof(char) * (57 + id_len));
    sprintf(msg, "[Error] [%s] Permission denied. Can't open Page Statistics",
            user.id);
    ReturnHistory(history_list->dummy_tail->pre, msg);
    return;
  }

  List *book = NewList(), *category = NewList();
  if (ErrorHandle(Filter(book, "", BOOK), 0)) {
    DeleteList(book, free);
    DeleteList(category, free);
    return;
  }
  for (ListNode *cur_node = book->dummy_head->nxt; cur_node != book->dummy_tail;
       cur_node = cur_node->nxt) {
    char *str = malloc(sizeof(char) *
                       (strlen(((Book *)cur_node->value)->category) + 1));
    strcpy(str, ((Book *)cur_node->value)->category);
    InsertList(category, category->dummy_tail, str);
  }
  DeleteList(book, free);
  SortList(category, StrLess);
  UniqueList(category, StrSame, free);

  char *str = malloc(sizeof(char) * 4);
  sprintf(str, "ALL");
  InsertList(category, category->dummy_head->nxt, str);

  List *borrow_record = NewList();
  if (ErrorHandle(Filter(borrow_record, "", BORROWRECORD), 0)) {
    DeleteList(category, free);
    DeleteList(borrow_record, free);
    return;
  }

  History *const new_history = malloc(sizeof(History));
  new_history->page = kStatistics;
  new_history->state.statistics = malloc(sizeof(Statistics));
  new_history->state.statistics->select_callback = Statistics_SelectCallback;
  new_history->state.statistics->turn_page = Statistics_TurnPage;
  new_history->state.statistics->catalogs = category;
  new_history->state.statistics->catalogs_start = category->dummy_head->nxt;
  new_history->state.statistics->borrow_record = borrow_record;
  new_history->state.statistics->borrow_record_start =
      borrow_record->dummy_head->nxt;
  PushBackHistory(new_history);

  if (!msg) {
    msg = malloc(sizeof(char) * (31 + id_len));
    sprintf(msg, "[Info] [%s] Open Page Statistics", user.id);
  }
  Log(msg);
  DrawUI(kStatistics, &user, new_history->state.statistics, msg);
}

static inline void Navigation_Return(char *msg) {
  if (history_list->size < 2) {
    if (!msg) {
      msg = malloc(sizeof(char) * (44 + id_len));
      sprintf(msg, "[Error] [%s] There's no history to go back to",
              user.id);
    }
    ReturnHistory(history_list->dummy_tail->pre, msg);
  } else {
    if (!msg) {
      msg = malloc(sizeof(char) * (18 + id_len));
      sprintf(msg, "[Info] [%s] Go back", user.id);
    }
    ReturnHistory(history_list->dummy_tail->pre->pre, msg);
  }
}

static inline void Navigation_Exit() {
  // TODO:(TO/GA) template那边有没有要处理的？
  Navigation_SaveLibrary(0, NULL);

  char *command = malloc(sizeof(char) * (14 + MAX_PATH + 8 + MAX_PATH + 12));

  if (ErrorHandle(CloseDBConnection(USER), 1, DB_NOT_OPEN)) {
    free(command);
    exit(1);
  }
  sprintf(command, "del /F %s", user_db_dir);
  system(command);

  if (ErrorHandle(CloseDBConnection(BOOK), 1, DB_NOT_OPEN)) {
    free(command);
    exit(1);
  }
  sprintf(command, "del /F %s", book_db_dir);
  system(command);

  if (ErrorHandle(CloseDBConnection(BORROWRECORD), 1, DB_NOT_OPEN)) {
    free(command);
    exit(1);
  }
  sprintf(command, "del /F %s", borrowrecord_db_dir);
  system(command);

  free(command);

  ClearHistory();
  DeleteList(history_list, free);

  Log("[Info] Shutdown");
  fclose(log_file);

  exit(0);
}

void NavigationCallback(Page nav_page) {
  switch (nav_page) {
    // case kWelcome: // 欢迎界面
    // break;
    case kLendAndBorrow:  // 借还书
      Navigation_LendAndBorrow(NULL);
      break;
    case kBookSearch:  // 图书搜索
      Navigation_BookSearch(NULL);
      break;
    case kUserSearch:  // 用户搜索（管理员）
      Navigation_UserSearch(NULL);
      break;
    case kManual:  // 帮助
      Navigation_ManualOrAbout(0, NULL);
      break;
    case kAbout:  // 关于
      Navigation_ManualOrAbout(1, NULL);
      break;
    case kUserRegister:  // 用户注册
      Navigation_UserLogInOrRegister(1, NULL);
      break;
    case kUserLogIn:  // 用户登陆
      Navigation_UserLogInOrRegister(0, NULL);
      break;
    case kLogout:  // 用户登出
      Navigation_UserLogOut(NULL);
      break;
    case kUserModify:  // 用户信息修改
      Navigation_UserModify(NULL);
      break;
    case kUserManagement:  // 用户删除/审核（管理员）
      Navigation_UserManagement(NULL);
      break;
    case kLibrary:  // 图书库显示
      Navigation_Library(NULL);
      break;
    case kInitLibrary:  // 图书库新建
      Navigation_OpenOrInitLibrary(1, NULL);
      break;
    case kOpenLibrary:  // 图书库打开
      Navigation_OpenOrInitLibrary(0, NULL);
      break;
    case kSaveLibrary:  // 图书库保存
      Navigation_SaveLibrary(1, NULL);
      break;
    // case kBookDisplay:  // 图书显示
    // break;
    case kBookInit:  // 图书新增
      Navigation_BookInit(NULL);
      break;
    // case kBookModify:  // 图书修改/删除
    // break;
    // case kBorrowDisplay:  // 借还书统计（管理员）
    // break;
    case kStatistics:  // 统计
      Navigation_Statistics(NULL);
      break;
    case kReturn:  // 回到上一个界面
      Navigation_Return(NULL);
      break;
    case kExit:  // 退出程序
      Navigation_Exit();
      break;
    default:
      Log("[Debug] Unknown nav_page in NavigationCallback");
      Error("Unknown nav_page");
  }
}

static inline void ReturnHistory(ListNode *go_back_to, char *msg) {
  while (history_list->size > 0 && history_list->dummy_tail->pre != go_back_to)
    PopBackHistory();
  if (history_list->size == 0) {
    History *const history = malloc(sizeof(History));
    history->page = kWelcome;
    DrawUI(kWelcome, &user, NULL, msg);
    return;
  }
  History *const history = go_back_to->value;
  switch (history->page) {
    case kWelcome:  // 欢迎界面
      Log(msg);
      DrawUI(kWelcome, &user, NULL, msg);
      break;
    case kLendAndBorrow:  // 借还书
      PopBackHistory();
      Navigation_LendAndBorrow(msg);
      break;
    case kBookSearch: {  // 图书搜索
      char *keyword =
          malloc(sizeof(char) * (strlen(history->state.book_search->keyword) + 1));
      strcpy(keyword, history->state.book_search->keyword);
      PopBackHistory();
      BookSearchDisplay(keyword, msg);
    } break;
    case kUserSearch: {  // 用户搜索（管理员）
      char *keyword =
          malloc(sizeof(char) * (strlen(history->state.user_search->keyword) + 1));
      strcpy(keyword, history->state.user_search->keyword);
      PopBackHistory();
      UserSearchDisplay(keyword, msg);
    } break;
    case kManual:  // 帮助
      PopBackHistory();
      Navigation_ManualOrAbout(0, msg);
      break;
    case kAbout:  // 关于
      PopBackHistory();
      Navigation_ManualOrAbout(1, msg);
      break;
    case kUserRegister:  // 用户注册
      PopBackHistory();
      Navigation_UserLogInOrRegister(1, msg);
      break;
    case kUserLogIn:  // 用户登陆
      PopBackHistory();
      Navigation_UserLogInOrRegister(0, msg);
      break;
    // case kLogout:  // 用户登出
    // break;
    case kUserModify: {  // 用户信息修改
      User *new_user = malloc(sizeof(User));
      if (ErrorHandle(GetById(new_user, history->state.user_modify->user->uid, USER), 0)) return;
      PopBackHistory();
      UserSearchInfoDisplay(new_user, msg);
      free(new_user);
    } break;
    case kUserManagement:  // 用户删除/审核（管理员）
      PopBackHistory();
      Navigation_UserManagement(msg);
      break;
    case kLibrary:  // 图书库显示
      PopBackHistory();
      Navigation_Library(msg);
      break;
    // case kInitLibrary:  // 图书库新建
    // break;
    // case kOpenLibrary:  // 图书库打开
    // break;
    // case kSaveLibrary:  // 图书库保存
    // break;
    case kBookDisplay:   // 图书显示
    case kBookModify: {  // 图书修改/删除
      Book *new_book = malloc(sizeof(Book));
      memcpy(new_book, history->state.book_display->book, sizeof(Book));
      PopBackHistory();
      Navigation_BookDisplayOrInit(new_book, 0, msg);
      free(new_book);
    } break;
    case kBookInit:  // 图书新增
      PopBackHistory();
      Navigation_BookInit(msg);
      break;
    case kBorrowDisplay:  // 借还书统计（管理员）
      PopBackHistory();
      BookDisplayAdminDisplay(msg);
      break;
    case kStatistics:  // 统计
      PopBackHistory();
      Navigation_Statistics(msg);
      break;
    // case kReturn:  // 回到上一个界面
    // break;
    // case kExit:  // 关闭程序
    // break;
    default:
      Log("[Debug] Unknown page in ReturnHistory");
      Error("Unknown nav_page");
  }
}
