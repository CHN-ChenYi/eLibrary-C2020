#include "view.h"

#include <io.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "basictype.h"
#include "exception.h"
#include "extgraph.h"
#include "graphics.h"
#include "gui.h"
#include "hash.h"
#include "list.h"
#include "model.h"

void DrawUI(Page cur_page, User *cur_user, void *info, char *terminal) {}

typedef struct History {
  Page page;
  State state;
} History;

static List *history_list;
static User user;
static char lib_path[MAX_PATH + 1], program_path[MAX_PATH + 1];
static size_t lib_path_len, username_len, program_path_len;
static char book_db_dir[MAX_PATH + 1], user_db_dir[MAX_PATH + 1],
    borrowrecord_db_dir[MAX_PATH + 1];
static FILE *log_file;
static inline void Log(char *const msg);
static inline char *MoveInList(ListNode **const node, List *list, int max_size,
                               bool direction, const char *const list_name,
                               const char *const page_name);
static inline bool ErrorHandle(int errno_);  // FALSE for success
static void FreeHistory(void *const history_);
static inline History *const TopHistory();
static inline void PushBackHistory(History *const new_history);
static inline void PopBackHistory();
static inline void ClearHistory();
static inline void ReturnHistory(ListNode *go_back_to, char *msg);
static void BookSearch_BorrowCallback(Book *book);
static void inline BookSearchDisplay(char *keyword, char *msg);
static void BookSearch_SearchCallback(char *keyword);
static void BookSearch_TurnPage(bool direction);
static void LendAndBorrow_SearchCallback(char *keyword);
static void LendAndBorrow_ReturnCallback(ListNode *book,
                                         ListNode *borrow_record);
static void LendAndBorrow_TurnPage(bool direction);
static void UserModify_ConfirmCallback();
static void UserModify_TurnPage(bool direction);
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
// TODO:(TO/GA) *_start 初始化
void Init() {
  InitConsole();               // TODO:(TO/GA) 可以删掉了？
  InitGraphics();              // TODO:(TO/GA) 可以删掉了？
  SetWindowTitle("eLibrary");  // TODO:(TO/GA) 可以删掉了？

  // init history
  history_list = NewList();
  History *const new_history = malloc(sizeof(History));
  new_history->page = kWelcome;
  PushBackHistory(new_history);

  // init log
  fopen_s(&log_file, ".\\eLibrary.log", "a+");

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
}

static inline char *MoveInList(ListNode **const node, List *list, int max_size,
                               bool direction, const char *const list_name,
                               const char *const page_name) {
  char *msg = malloc(sizeof(char) * (59 + username_len + strlen(list_name) +
                                     strlen(page_name)));
  if (direction) {
    ListNode *new_node = *node;
    for (int i = max_size; i && new_node != list->dummy_tail; i--)
      new_node = new_node->nxt;
    if (new_node == list->dummy_tail) {
      sprintf(
          msg,
          "[Error] [%s] Fail to turn to the next page in List %s of Page %s",
          user.username, list_name, page_name);
    } else {
      *node = new_node;
      sprintf(msg, "[Info] [%s] Turn to the next page in List %s of Page %s",
              user.username, list_name, page_name);
    }
  } else {
    for (int i = max_size; i; i--) *node = (*node)->pre;
    sprintf(msg, "[Info] [%s] Turn to the prev page in List %s of Page %s",
            user.username, list_name, page_name);
  }
  return msg;
}

static inline bool ErrorHandle(int errno_) {
  char *msg = malloc(sizeof(char) * (41 + username_len));
  switch (errno_) {
    case DB_SUCCESS:
      free(msg);
      return FALSE;
    case DB_NOT_FOUND:
      sprintf(msg, "[Error] [%s] DB_NOT_FOUND", user.username);
      break;
    case DB_NOT_OPEN:
      sprintf(msg, "[Error] [%s] DB_NOT_OPEN", user.username);
      break;
    case DB_NOT_CLOSE:
      sprintf(msg, "[Error] [%s] DB_NOT_CLOSE", user.username);
      break;
    case DB_NOT_EXISTS:
      sprintf(msg, "[Error] [%s] DB_NOT_EXISTS", user.username);
      break;
    case DB_FAIL_ON_INIT:
      sprintf(msg, "[Error] [%s] DB_FAIL_ON_INIT", user.username);
      break;
    case DB_FAIL_ON_FETCHING:
      sprintf(msg, "[Error] [%s] DB_FAIL_ON_FETCHING", user.username);
      break;
    case DB_FAIL_ON_WRITING:
      sprintf(msg, "[Error] [%s] DB_FAIL_ON_WRITING", user.username);
      break;
    case DB_FAIL_ON_CREATE:
      sprintf(msg, "[Error] [%s] DB_FAIL_ON_CREATE", user.username);
      break;
    case DB_FAIL_ON_UPDATE:
      sprintf(msg, "[Error] [%s] DB_FAIL_ON_UPDATE", user.username);
      break;
    case DB_FAIL_ON_DELETE:
      sprintf(msg, "[Error] [%s] DB_FAIL_ON_DELETE", user.username);
      break;
    case DB_FAIL_ON_GETTING_PROPERTIES:
      sprintf(msg, "[Error] [%s] DB_FAIL_ON_GETTING_PROPERTIES", user.username);
      break;
    case DB_ALREADY_EXISTS:
      sprintf(msg, "[Error] [%s] DB_ALREADY_EXISTS", user.username);
      break;
    case DB_ENTRY_EMPTY:
      sprintf(msg, "[Error] [%s] DB_ENTRY_EMPTY", user.username);
      break;
    case DB_FAIL:
      sprintf(msg, "[Error] [%s] DB_FAIL", user.username);
      break;
    default:
      Log("[Debug] Unknown errno");
      Error("Unknown errno");
  }
  ReturnHistory(history_list->dummy_tail->pre, msg);
  return TRUE;
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
      DeleteList(history->state.user_modify->books, free);
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
      free(history->state.borrow_display->book_name);
      break;
    case kBookInit:    // 图书新增
    case kBookModify:  // 图书修改/删除
      free(history->state.book_display->book);
      break;
    case kBorrowDisplay:  // 借还书统计（管理员）
      free(history->state.borrow_display->book_name);
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
        malloc(sizeof(char) * (38 + username_len + strlen(book->title)));
    sprintf(msg, "[Error] [%s] There's no [%s] on the shelf", user.username,
            book->title);
    ReturnHistory(history_list->dummy_tail->pre, msg);
    return;
  }
  book->number_on_the_shelf--;
  if (ErrorHandle(Update(book, book->uid, BOOK))) return;

  BorrowRecord new_record;
  strcpy(new_record.book_name, book->title);
  new_record.book_status = BORROWED;
  new_record.book_uid = book->uid;
  time_t now_time_t = time(0);
  struct tm *now_tm = localtime(&now_time_t);
  sprintf(new_record.borrowed_date, "%04d%02d%02d", now_tm->tm_year + 1900,
          now_tm->tm_mon + 1, now_tm->tm_mday);
  time_t nxt_time_t =
      now_time_t + (time_t)86400 * book->available_borrowed_days;
  struct tm *nxt_tm = localtime(&nxt_time_t);
  sprintf(new_record.returned_date, "%04d%02d%02d\n", nxt_tm->tm_year + 1900,
          nxt_tm->tm_mon + 1, nxt_tm->tm_mday);
  if (ErrorHandle(GetNextPK(BORROWRECORD, &new_record.uid))) return;
  strcpy(new_record.user_name, user.username);
  new_record.user_uid = user.uid;
  if (ErrorHandle(Create(&new_record, BORROWRECORD))) return;

  char *msg = malloc(sizeof(char) * (25 + username_len + strlen(book->title)));
  sprintf(msg, "[Info] [%s] Borrow book [%s]", user.username, book->title);
  ReturnHistory(history_list->dummy_tail->pre, msg);
}

static void inline BookSearchDisplay(char *keyword, char *msg) {
  List *results = NewList();
  if (ErrorHandle(Filter(results, keyword, BOOK))) return;

  History *const new_history = malloc(sizeof(History));
  new_history->page = kBookSearch;
  new_history->state.book_search = malloc(sizeof(BookSearch));
  new_history->state.book_search->keyword = keyword;
  new_history->state.book_search->borrow_callback = BookSearch_BorrowCallback;
  new_history->state.book_search->search_callback = BookSearch_SearchCallback;
  new_history->state.book_search->turn_page = BookSearch_TurnPage;
  new_history->state.book_search->book_result = results;
  new_history->state.book_search->book_result_start = results->dummy_head->nxt;
  PushBackHistory(new_history);

  if (!msg) {
    msg = malloc(sizeof(char) * (23 + username_len + strlen(keyword)));
    sprintf(msg, "[Info] [%s] Book search %s", user.username, keyword);
  }
  Log(msg);
  DrawUI(kBookSearch, &user, new_history->state.book_search, msg);
}

static void BookSearch_SearchCallback(char *keyword) {
  BookSearchDisplay(keyword, NULL);
}

static void BookSearch_TurnPage(bool direction) {
  BookSearch *state = TopHistory()->state.book_search;
  char *msg = MoveInList(&state->book_result_start, state->book_result,
                         kBookSearchMax, direction, "Result", "Book Search");
  DrawUI(kBookSearch, &user, state, msg);
}

static void LendAndBorrow_SearchCallback(char *keyword) {
  BookSearch_SearchCallback(keyword);
}

static void LendAndBorrow_ReturnCallback(ListNode *book,
                                         ListNode *borrow_record) {
  Book *returned_book = (Book *)book->value;
  BorrowRecord *returned_borrow_record = (BorrowRecord *)borrow_record->value;

  returned_book->number_on_the_shelf++;
  if (ErrorHandle(Update(returned_book, returned_book->uid, BOOK))) return;

  char *msg = malloc(sizeof(char) * (49 + strlen(returned_book->title) + 8));
  sprintf(msg, "[Info] [%s] Return book [%s], expected return date[%s]",
          user.username, returned_book->title,
          returned_borrow_record->returned_date);

  returned_borrow_record->book_status = RETURNED;
  time_t now_time_t = time(0);
  struct tm *now_tm = localtime(&now_time_t);
  sprintf(returned_borrow_record->returned_date, "%04d%02d%02d",
          now_tm->tm_year + 1900, now_tm->tm_mon + 1, now_tm->tm_mday);
  if (ErrorHandle(Update(returned_borrow_record, returned_borrow_record->uid,
                         BORROWRECORD))) {
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
      char *msg = malloc(sizeof(char) * (56 + username_len));
      sprintf(msg, "[Error] [%s] Password incorrect. Can't modify user's info",
              user.username);
      ReturnHistory(history_list->dummy_tail->pre, msg);
      return;
    }
  }
  if (TopHistory()->state.user_modify->new_password[0] != '\0') {
    if (strcmp(TopHistory()->state.user_modify->new_password,
               TopHistory()->state.user_modify->repeat_password)) {
      char *msg = malloc(sizeof(char) * (58 + username_len));
      sprintf(msg,
              "[Error] [%s] Repeat new password doesn't match new password",
              user.username);
      ReturnHistory(history_list->dummy_tail->pre, msg);
      return;
    }
    char new_pwd[59];
    sprintf(new_pwd, "%s%s", TopHistory()->state.user_modify->new_password,
            modified_user->salt);
    Sha256Sum(modified_user->password, new_pwd, strlen(new_pwd));
  }

  List *users = NewList();
  char *query = malloc(sizeof(char) * (10 + strlen(modified_user->username)));
  sprintf(query, "username=%s", modified_user->username);
  free(query);
  if (users->size > 1 ||
      (users->size == 1 &&
       ((User *)users->dummy_head->nxt->value)->uid != modified_user->uid)) {
    DeleteList(users, free);
    char *msg = malloc(sizeof(char) * 42);
    sprintf(msg, "[Error] Fail to modify, username exists");
    ReturnHistory(history_list->dummy_tail->pre, msg);
    return;
  }
  DeleteList(users, free);

  if (modified_user->username[0] == '\0') {
    char *msg = malloc(sizeof(char) * (51 + username_len + 10));
    sprintf(msg, "[Error] [%s] User [uid = %d]'s username can't be blank",
            user.username, modified_user->uid);
    ReturnHistory(history_list->dummy_tail->pre, msg);
    return;
  }

  if (ErrorHandle(Update(modified_user, modified_user->uid, USER))) return;
  if (modified_user->uid == user.uid) {
    memcpy(&user, modified_user, sizeof(USER));
    username_len = strlen(user.username);
  }

  char *msg = malloc(sizeof(char) *
                     (32 + username_len + strlen(modified_user->username)));
  sprintf(msg, "[Info] [%s] Modify user [%s]'s info", user.username,
          modified_user->username);
  ReturnHistory(history_list->dummy_tail->pre, msg);
}

static void UserModify_TurnPage(bool direction) {
  UserModify *state = TopHistory()->state.user_modify;
  char *msg = MoveInList(&state->books_start, state->books, kUserModifyMax,
                         direction, "User", "User Modify");
  DrawUI(kUserModify, &user, state, msg);
}

static void inline UserSearchInfoDisplay(User *show_user, char *msg) {
  if (ErrorHandle(GetById(show_user, show_user->uid, USER))) return;

  // TODO:(TO/GA)按还书时间排序
  List *borrow_record = NewList(), *books = NewList();
  char *query = malloc(sizeof(char) * (10 + 10));
  sprintf(query, "user_uid=%d", show_user->uid);
  if (ErrorHandle(Filter(borrow_record, query, BORROWRECORD))) {
    free(query);
    DeleteList(borrow_record, free);
    DeleteList(books, free);
    return;
  }
  free(query);
  for (ListNode *cur_node = borrow_record->dummy_head->nxt;
       cur_node != borrow_record->dummy_tail; cur_node = cur_node->nxt) {
    Book *new_book = malloc(sizeof(BOOK));
    if (ErrorHandle(GetById(
            new_book, ((BorrowRecord *)cur_node->value)->book_uid, BOOK))) {
      free(new_book);
      DeleteList(borrow_record, free);
      DeleteList(books, free);
      return;
    }
    InsertList(books, books->dummy_tail, new_book);
  }

  History *const new_history = malloc(sizeof(History));
  new_history->page = kUserModify;
  new_history->state.user_modify = malloc(sizeof(UserModify));
  new_history->state.user_modify->confirm_callback = UserModify_ConfirmCallback;
  new_history->state.user_modify->turn_page = UserModify_TurnPage;
  new_history->state.user_modify->user = malloc(sizeof(User));
  memcpy(new_history->state.user_modify->user, show_user, sizeof(User));
  new_history->state.user_modify->books = books;
  new_history->state.user_modify->books_start = books->dummy_head->nxt;
  PushBackHistory(new_history);

  if (!msg) {
    msg = malloc(sizeof(char) *
                 (30 + username_len + strlen(show_user->username)));
    sprintf(msg, "[Info] [%s] Show/Modify user [%s]", user.username,
            show_user->username);
  }
  Log(msg);
  DrawUI(kUserModify, &user, new_history->state.user_modify, msg);
}

static void UserSearch_InfoCallback(User *show_user) {
  UserSearchInfoDisplay(show_user, NULL);
}

static void inline UserSearchDisplay(char *keyword, char *msg) {
  if (user.whoami != ADMINISTRATOR) {
    char *msg = malloc(sizeof(char) * (49 + username_len));
    sprintf(msg, "[Error] [%s] Permission denied. Can't search users",
            user.username);
    ReturnHistory(history_list->dummy_tail->pre, msg);
    return;
  }

  List *results = NewList();
  if (ErrorHandle(Filter(results, keyword, USER))) {
    DeleteList(results, free);
    return;
  }

  History *const new_history = malloc(sizeof(History));
  new_history->page = kBookSearch;
  new_history->state.user_search = malloc(sizeof(UserSearch));
  new_history->state.user_search->keyword = keyword;
  new_history->state.user_search->info_callback = UserSearch_InfoCallback;
  new_history->state.user_search->search_callback = UserSearch_SearchCallback;
  new_history->state.user_search->turn_page = UserSearch_TurnPage;
  new_history->state.user_search->user_result = results;
  new_history->state.user_search->user_result_start = results->dummy_head->nxt;
  PushBackHistory(new_history);

  if (!msg) {
    msg = malloc(sizeof(char) * (23 + username_len + strlen(keyword)));
    sprintf(msg, "[Info] [%s] User search %s", user.username, keyword);
  }
  Log(msg);
  DrawUI(kUserSearch, &user, new_history->state.user_search, msg);
}

static void UserSearch_SearchCallback(char *keyword) {
  UserSearchDisplay(keyword, NULL);
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
      char *msg = malloc(sizeof(char) * (47 + username_len));
      sprintf(msg, "[Error] Repeat password doesn't match password");
      ReturnHistory(history_list->dummy_tail->pre, msg);
      return;
    }

    User *new_user = TopHistory()->state.login_or_register->user;

    if (new_user->username[0] == '\0') {
      char *msg = malloc(sizeof(char) * 32);
      sprintf(msg, "[Error] username can't be blank");
      ReturnHistory(history_list->dummy_tail->pre, msg);
      return;
    }

    List *users = NewList();
    char *query = malloc(sizeof(char) * (10 + strlen(new_user->username)));
    sprintf(query, "username=%s", new_user->username);
    free(query);
    if (users->size != 0) {
      DeleteList(users, free);
      char *msg = malloc(sizeof(char) * 42);
      sprintf(msg, "[Error] Fail to register, username exists");
      ReturnHistory(history_list->dummy_tail->pre, msg);
      return;
    }
    DeleteList(users, free);

    if (ErrorHandle(GetNextPK(USER, &new_user->uid))) return;
    RandStr(new_user->salt, 9);
    char pwd_type[59];
    sprintf(pwd_type, "%s%s", TopHistory()->state.login_or_register->password,
            new_user->salt);
    Sha256Sum(new_user->password, pwd_type, strlen(pwd_type));

    unsigned size_of_user_db;
    if (ErrorHandle(GetDBSize(USER, &size_of_user_db))) return;

    char *msg = malloc(sizeof(char) * (45 + strlen(new_user->username)));

    if (!size_of_user_db) {  // the first user is admin
      new_user->whoami = ADMINISTRATOR;
      new_user->verified = TRUE;
      sprintf(msg, "[Info] [%s] Registered as an admin", new_user->username);
    } else {
      sprintf(msg, "[Info] [%s] Register. Wait for admin to verify",
              new_user->username);
      new_user->verified = FALSE;
    }
    if (ErrorHandle(Create(new_user, USER))) {
      free(msg);
      return;
    }

    History *const new_history = malloc(sizeof(History));
    new_history->page = kWelcome;
    PushBackHistory(new_history);

    Log(msg);
    DrawUI(kWelcome, &user, NULL, msg);
  } else {
    User *new_user = TopHistory()->state.login_or_register->user;

    List *users = NewList();
    char *query = malloc(sizeof(char) * (10 + strlen(new_user->username)));
    sprintf(query, "username=%s", new_user->username);
    free(query);
    if (users->size != 1) {
      DeleteList(users, free);
      char *msg = malloc(sizeof(char) * 31);
      sprintf(msg, "[Error] Can't find such a user");
      ReturnHistory(history_list->dummy_tail->pre, msg);
      return;
    }
    DeleteList(users, free);
    memcpy(new_user, users->dummy_head->nxt->value, sizeof(User));

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
      char *msg = malloc(sizeof(char) * (60 + strlen(new_user->username)));
      sprintf(msg,
              "[Error] [%s] You haven't been verified. Please contact admin",
              new_user->username);
      ReturnHistory(history_list->dummy_tail->pre, msg);
      return;
    }
    memcpy(&user, new_user, sizeof(User));
    username_len = strlen(user.username);

    History *const new_history = malloc(sizeof(History));
    new_history->page = kWelcome;
    PushBackHistory(new_history);

    char *msg = malloc(sizeof(char) * (17 + username_len));
    sprintf(msg, "[Info] [%s] Log in", user.username);
    Log(msg);
    DrawUI(kWelcome, &user, NULL, msg);
  }
}

static void UserManagement_ApproveCallback(ListNode *user_node, bool approve) {
  User *new_user = user_node->value;
  char *msg =
      malloc(sizeof(char) * (27 + username_len + strlen(new_user->username)));
  if (approve) {
    new_user->verified = TRUE;
    if (ErrorHandle(Update(new_user, new_user->uid, USER))) {
      free(msg);
      return;
    }
    sprintf(msg, "[Info] [%s] Approve user [%s]", user.username,
            new_user->username);
  } else {
    if (ErrorHandle(Delete(new_user->uid, USER))) {
      free(msg);
      return;
    }
    sprintf(msg, "[Info] [%s] Disprove user [%s]", user.username,
            new_user->username);
  }
  ReturnHistory(history_list->dummy_tail->pre, msg);
}

static void UserManagement_DeleteCallback(ListNode *user_node) {
  User *new_user = user_node->value;
  if (ErrorHandle(Delete(new_user->uid, USER))) return;
  char *msg =
      malloc(sizeof(char) * (25 + username_len + strlen(new_user->username)));
  sprintf(msg, "[Info] [%s] Delete user [%s]", user.username,
          new_user->username);
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
    char *msg = malloc(sizeof(char) * (60 + username_len));
    sprintf(msg,
            "[Error] [%s] Permission denied. Can't open Page BorrowDisplay",
            user.username);
    ReturnHistory(history_list->dummy_tail->pre, msg);
    return;
  }

  if (ErrorHandle(GetById(TopHistory()->state.book_display->book,
                          TopHistory()->state.book_display->book->uid, BOOK)))
    return;

  List *borrow_record = NewList();
  char *query = malloc(sizeof(char) * 20);
  sprintf(query, "book_uid=%d", TopHistory()->state.book_display->book->uid);
  if (ErrorHandle(Filter(borrow_record, query, BORROWRECORD))) {
    free(query);
    DeleteList(borrow_record, free);
    return;
  }
  free(query);
  // TODO:(TO/GA) 排序
  History *const new_history = malloc(sizeof(History));
  new_history->page = kBorrowDisplay;
  new_history->state.borrow_display = malloc(sizeof(BorrowDisplay));
  strcpy(new_history->state.borrow_display->book_name,
         TopHistory()->state.book_display->book->title);
  new_history->state.borrow_display->turn_page = BorrowDisplay_TurnPage;
  new_history->state.borrow_display->borrow_record = borrow_record;
  new_history->state.borrow_display->borrow_record_start =
      borrow_record->dummy_head->nxt;
  PushBackHistory(new_history);

  if (!msg) {
    msg = malloc(sizeof(char) *
                 (46 + username_len +
                  strlen(new_history->state.borrow_display->book_name)));
    sprintf(msg, "[Info] [%s] Open Page BorrowDisplay for book [%s]",
            user.username, new_history->state.borrow_display->book_name);
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
      char *msg = malloc(sizeof(char) * (34 + username_len));
      sprintf(msg, "[Error] [%s] Fail to open the image", user.username);
      DrawUI(kBookDisplay, &user, TopHistory()->state.book_display, msg);
      return;
    }
  }
  endtry;
  const int uid = TopHistory()->state.book_display->book->uid;
  char *command =
      malloc(sizeof(char) * (25 + sizeof(image_path) + lib_path_len + 10));
  sprintf(command, "copy /Y \"%s\" \"%s\\image\\%d.jpg\"", image_path, lib_path,
          uid);

  char *msg = malloc(sizeof(char) * (51 + username_len + 10));
  if (system(command)) {
    sprintf(msg, "[Error] [%s] Fail to change the book(uid = %d)'s cover",
            user.username, uid);
    free(command);
    ReturnHistory(history_list->dummy_tail->pre, msg);
    return;
  }
  sprintf(msg, "[Info] [%s] Change the book(uid = %d)'s cover", user.username,
          uid);
  Log(msg);
  DrawUI(kBookDisplay, &user, TopHistory()->state.book_display, msg);
  free(command);
}

static void BookDisplay_ConfirmCallback() {
  if (user.whoami != ADMINISTRATOR) {
    char *msg = malloc(sizeof(char) * (60 + username_len));
    sprintf(msg, "[Error] [%s] Permission denied. Can't modify any book",
            user.username);
    ReturnHistory(history_list->dummy_tail->pre, msg);
    return;
  }

  Book *new_book = TopHistory()->state.book_display->book;
  if (new_book->id[0] == '\0') {
    char *msg = malloc(sizeof(char) * (36 + username_len));
    sprintf(msg, "[Error] [%s] Book's id can't be blank", user.username);
    ReturnHistory(history_list->dummy_tail->pre, msg);
    return;
  }
  if (new_book->title[0] == '\0') {
    char *msg = malloc(sizeof(char) * (39 + username_len));
    sprintf(msg, "[Error] [%s] Book's title can't be blank", user.username);
    ReturnHistory(history_list->dummy_tail->pre, msg);
    return;
  }
  if (ErrorHandle(Update(new_book, new_book->uid, BOOK))) return;

  char *msg =
      malloc(sizeof(char) * (25 + username_len + strlen(new_book->title)));
  sprintf(msg, "[Info] [%s] Modify book [%s]", user.username, new_book->title);
  ReturnHistory(history_list->dummy_tail->pre, msg);
}

static void BookDisplay_DeleteCallback() {
  if (user.whoami != ADMINISTRATOR) {
    char *msg = malloc(sizeof(char) * (60 + username_len));
    sprintf(msg, "[Error] [%s] Permission denied. Can't modify any book",
            user.username);
    ReturnHistory(history_list->dummy_tail->pre, msg);
    return;
  }

  Book *new_book = TopHistory()->state.book_display->book;
  if (ErrorHandle(Delete(new_book->uid, BOOK))) return;

  char *msg =
      malloc(sizeof(char) * (25 + username_len + strlen(new_book->title)));
  sprintf(msg, "[Info] [%s] Delete book [%s]", user.username, new_book->title);
  ReturnHistory(history_list->dummy_tail->pre, msg);
}

static void BookDisplay_BorrowCallback() {
  BookSearch_BorrowCallback(TopHistory()->state.book_display->book);
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
  char *msg = malloc(sizeof(char) * (33 + username_len));
  switch (sort_keyword) {
    case kId:
      SortList(TopHistory()->state.library->books, CmpById);
      sprintf(msg, "[Info] [%s] sort books by id", user.username);
      break;
    case kTitle:
      SortList(TopHistory()->state.library->books, CmpByTitle);
      sprintf(msg, "[Info] [%s] sort books by title", user.username);
      break;
    case kAuthor:
      SortList(TopHistory()->state.library->books, CmpByAuthor);
      sprintf(msg, "[Info] [%s] sort books by author", user.username);
      break;
    default:
      Log("[Debug] Unknown sort_keyword in Library_SortCallback");
      Error("Unknown nav_page");
      break;
  }
  Log(msg);
  DrawUI(kLibrary, &user, TopHistory()->state.library, msg);
}

static void Library_SwitchCallback() {
  if (TopHistory()->state.library->type == kList) {
    Navigation_Library(NULL);
  } else {
    List *books = NewList();
    if (ErrorHandle(Filter(books, "", BOOK))) {
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

    char *msg = malloc(sizeof(char) * (41 + username_len));
    sprintf(msg, "[Info] [%s] Open Page Library (list mode)", user.username);
    Log(msg);
    DrawUI(kLibrary, &user, new_history->state.library, msg);
  }
}

void Library_TurnPage(bool direction) {
  Library *state = TopHistory()->state.library;
  MoveInList(&state->books_start, state->books, kLibraryMax, direction, "", "");
  char *msg = MoveInList(&state->books_covers_start, state->book_covers,
                         kLibraryMax, direction, "Book", "Library");
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
  if (ErrorHandle(Filter(borrow_records, "", BORROWRECORD))) {
    DeleteList(borrow_records, free);
    return;
  }
  for (const ListNode *cur_node = borrow_records->dummy_head->nxt;
       cur_node != borrow_records->dummy_tail;) {
    if (ErrorHandle(
            GetById(book, ((BorrowRecord *)cur_node->value)->book_uid, BOOK))) {
      DeleteList(borrow_records, free);
      free(book);
      return;
    }
    if (strcmp(book->category, catalog->value))
      cur_node = EraseList(borrow_records, cur_node, NULL);
    else
      cur_node = cur_node->nxt;
  }

  History *const new_history = malloc(sizeof(History));
  new_history->page = kStatistics;
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
      malloc(sizeof(char) * (46 + username_len + strlen(catalog->value)));
  sprintf(msg, "[Info] [%s] Open Page Statistics with category %s",
          user.username, (char *)catalog->value);
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

static inline void Navigation_LendAndBorrow(char *msg) {
  List *borrow_records_list = NewList();
  char *query = malloc(sizeof(char) * (31 + 10));
  sprintf(query, "user_uid=%d&book_status=BORROWED", user.uid);
  if (ErrorHandle(Filter(borrow_records_list, query, USER))) {
    DeleteList(borrow_records_list, free);
    free(query);
    return;
  }
  free(query);
  // TODO:(TO/GA)按还书时间排序

  List *books = NewList();
  for (ListNode *cur_node = borrow_records_list->dummy_head;
       cur_node != borrow_records_list->dummy_tail; cur_node = cur_node->nxt) {
    Book *book = malloc(sizeof(Book));
    if (ErrorHandle(GetById(book, ((BorrowRecord *)cur_node->value)->book_uid,
                            BORROWRECORD))) {
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
    msg = malloc(sizeof(char) * (34 + username_len));
    sprintf(msg, "[Info] [%s] Open Page LendAndBorrow", user.username);
  }
  Log(msg);
  DrawUI(kLendAndBorrow, &user, new_history->state.lend_and_borrow, msg);
}

static inline void Navigation_BookSearch(char *msg) {
  BookSearchDisplay(NULL, msg);
}

static inline void Navigation_UserSearch(char *msg) {
  UserSearchDisplay(NULL, msg);
}

// type = 0 => Manual
static inline void Navigation_ManualOrAbout(bool type, char *msg) {
  History *const new_history = malloc(sizeof(History));
  new_history->state.manual_and_about = malloc(sizeof(ManualAndAbout));
  // TODO:(TO/GA) finish it
  if (type) {
    new_history->page = kAbout;
    // new_history->state.manual_and_about->img
  } else {
    new_history->page = kManual;
    // new_history->state.manual_and_about->img
  }
  PushBackHistory(new_history);

  if (!msg) {
    msg = malloc(sizeof(char) * (27 + username_len));
    if (type)
      sprintf(msg, "[Info] [%s] Open Page About", user.username);
    else
      sprintf(msg, "[Info] [%s] Open Page Manual", user.username);
  }
  Log(msg);
  DrawUI(kAbout, &user, new_history->state.manual_and_about, msg);
}

// type = 0 => LogIn
static inline void Navigation_UserLogInOrRegister(bool type, char *msg) {
  memset(&user, 0x00, sizeof(User));

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
  ClearHistory();
  History *const new_history = malloc(sizeof(History));
  new_history->page = kWelcome;
  PushBackHistory(new_history);

  if (!msg) {
    msg = malloc(sizeof(char) * (36 + username_len));
    sprintf(msg, "[Info] [%s] Clear history and log out", user.username);
  }

  memset(&user, 0x00, sizeof(User));
  username_len = 0;

  Log(msg);
  DrawUI(kWelcome, &user, new_history->state.login_or_register, msg);
}

static inline void Navigation_UserModify(char *msg) {
  UserSearchInfoDisplay(&user, msg);
}

static inline void Navigation_UserManagement(char *msg) {
  if (user.whoami != ADMINISTRATOR) {
    char *msg = malloc(sizeof(char) * (49 + username_len));
    sprintf(msg, "[Error] [%s] Permission denied. Can't manage users",
            user.username);
    ReturnHistory(history_list->dummy_tail->pre, msg);
    return;
  }

  List *to_be_verified = NewList();
  if (ErrorHandle(Filter(to_be_verified, "verified=FALSE", USER))) {
    DeleteList(to_be_verified, free);
    return;
  }

  List *verified = NewList();
  if (ErrorHandle(Filter(verified, "verified=TRUE", USER))) {
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
    msg = malloc(sizeof(char) * (36 + username_len));
    sprintf(msg, "[Info] [%s] Open Page User Management", user.username);
  }
  Log(msg);
  DrawUI(kUserManagement, &user, new_history->state.user_management, msg);
}

static inline void Navigation_Library(char *msg) {
  List *books = NewList();
  if (ErrorHandle(Filter(books, "", BOOK))) {
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
    sprintf(image_path + image_path_len, "%d.jpg",
            ((Book *)cur_node->value)->uid);
    loadImage(image_path, image);
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
    msg = malloc(sizeof(char) * (41 + username_len));
    sprintf(msg, "[Info] [%s] Open Page Library (image mode)", user.username);
  }
  Log(msg);
  DrawUI(kLibrary, &user, new_history->state.library, msg);
}

// type == 0 => Open
static inline void Navigation_OpenOrInitLibrary(bool type, char *msg) {
  try {
    SelectFolder("请选择保存图书库的文件夹", lib_path);
    except(ErrorException) {
      char *msg = malloc(sizeof(char) * (56 + username_len));
      if (type)
        sprintf(msg,
                "[Error] [%s] Fail to init the library. Path doesn't exist",
                user.username);
      else
        sprintf(msg,
                "[Error] [%s] Fail to open the library. Path doesn't exist",
                user.username);
      ReturnHistory(history_list->dummy_tail->pre, msg);
      return;
    }
  }
  endtry;
  lib_path_len = strlen(lib_path);

  // copy *.swp.db to *.db and remove *.swp.db
  Navigation_SaveLibrary(0, NULL);
  char *command = malloc(sizeof(char) * (8 + strlen(borrowrecord_db_dir)));

  if (ErrorHandle(CloseDBConnection(USER))) {
    free(command);
    return;
  }
  sprintf(command, "del /F %s", user_db_dir);
  system(command);

  if (ErrorHandle(CloseDBConnection(BOOK))) {
    free(command);
    return;
  }
  sprintf(command, "del /F %s", book_db_dir);
  system(command);

  if (ErrorHandle(CloseDBConnection(BORROWRECORD))) {
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
      char *msg = malloc(sizeof(char) * (63 + username_len));
      sprintf(
          msg,
          "[Error] [%s] Fail to init the library. Can't create image folder",
          user.username);
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
    if (!_access(user_database_path, 6)) {  // book database exists
      char *command =
          malloc(sizeof(char) * (14 + lib_path_len + 8 + lib_path_len + 12));
      sprintf(command, "copy /Y \"%s\" \"%s\"", user_database_path,
              user_db_dir);
      system(command);
      free(command);
    } else {  // book database doesn't exist
      flag |= 2;
    }
    free(user_database_path);
  }
  if (ErrorHandle(OpenDBConnection(user_db_dir, USER))) return;

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
      flag |= 2;
    }
    free(book_database_path);
  }
  if (ErrorHandle(OpenDBConnection(book_db_dir, BOOK))) return;

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
          malloc(sizeof(char) * (14 + lib_path_len + 8 + lib_path_len + 12));
      sprintf(command, "copy /Y \"%s\" \"%s\"", borrowrecord_database_path,
              borrowrecord_db_dir);
      system(command);
      free(command);
    } else {  // borrowrecord database doesn't exist
      flag |= 2;
    }
    free(borrowrecord_database_path);
  }
  if (ErrorHandle(OpenDBConnection(borrowrecord_db_dir, BORROWRECORD))) return;

  ClearHistory();
  History *const new_history = malloc(sizeof(History));
  new_history->page = kWelcome;
  PushBackHistory(new_history);

  if (!msg) {
    msg = malloc(sizeof(char) * (114 + lib_path_len + username_len));
    if (type) {
      if (flag)
        sprintf(msg,
                "[Error] [%s] Log out, Clear history and init library from %s, "
                "where "
                "already exists an eLibrary",
                user.username, lib_path);
      else
        sprintf(msg,
                "[Info] [%s] Log out, Clear history and init library from %s",
                user.username, lib_path);
    } else {
      if ((flag & 2) == 2)
        sprintf(msg,
                "[Error] [%s] Log out, Clear history and fail to open library "
                "from %s, "
                "init one at there, undefined behavior may occur",
                user.username, lib_path);
      else if ((flag & 1) == 1)
        sprintf(msg,
                "[Warning] [%s] Log out, Clear history and open library from "
                "%s using "
                "swap file",
                user.username, lib_path);
      else if (!flag)
        sprintf(msg,
                "[Info] [%s] Log out, Clear history and open library from %s",
                user.username, lib_path);
    }
  }

  memset(&user, 0x00, sizeof(User));
  username_len = 0;

  Log(msg);
  DrawUI(kWelcome, &user, NULL, msg);
}

// type = 0 => 不回退到上一个界面
static inline void Navigation_SaveLibrary(bool type, char *msg) {
  char *command =
      malloc(sizeof(char) * (14 + lib_path_len + 16 + lib_path_len + 20));

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

  if (type) {
    if (!msg) {
      msg = malloc(sizeof(char) * (23 + username_len));
      sprintf(msg, "[Info] [%s] Save library", user.username);
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
    if (ErrorHandle(GetById(new_book, book->uid, BOOK))) {
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
  new_history->state.book_display->book = new_book;
  if (!type) {
    char *image_path = malloc(sizeof(char) * (12 + lib_path_len + 10));
    sprintf(image_path, "%s\\image\\%d.jpg", lib_path, book->uid);
    loadImage(image_path, &new_history->state.book_display->book_cover);
  }
  PushBackHistory(new_history);

  if (!msg) {
    msg = malloc(sizeof(char) * (33 + username_len));
    if (type) {
      sprintf(msg, "[Info] [%s] Open book init page", user.username);
    } else {
      if (user.whoami == ADMINISTRATOR)
        sprintf(msg, "[Info] [%s] Open book modify page", user.username);
      else
        sprintf(msg, "[Info] [%s] Open book display page", user.username);
    }
  }
  Log(msg);
  DrawUI(new_history->page, &user, new_history->state.book_display, msg);
}

static inline void Navigation_BookInit(char *msg) {
  Book *book = malloc(sizeof(Book));
  if (ErrorHandle(GetNextPK(BOOK, &book->uid))) {
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
  List *book = NewList(), *category = NewList();
  if (ErrorHandle(Filter(book, "", BOOK))) {
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

  List *borrow_record = NewList();
  if (ErrorHandle(Filter(borrow_record, "", BORROWRECORD))) {
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
    msg = malloc(sizeof(char) * (31 + username_len));
    sprintf(msg, "[Info] [%s] Open Page Statistics", user.username);
  }
  Log(msg);
  DrawUI(kStatistics, &user, new_history->state.statistics, msg);
}

static inline void Navigation_Return(char *msg) {
  if (history_list->size < 2) {
    if (!msg) {
      msg = malloc(sizeof(char) * (44 + username_len));
      sprintf(msg, "[Error] [%s] There's no history to go back to",
              user.username);
    }
    ReturnHistory(history_list->dummy_tail->pre, msg);
  } else {
    if (!msg) {
      msg = malloc(sizeof(char) * (18 + username_len));
      sprintf(msg, "[Info] [%s] Go back", user.username);
    }
    ReturnHistory(history_list->dummy_tail->pre->pre, msg);
  }
}

static inline void Navigation_Exit() {
  // TODO:(TO/GA) template那边有没有要处理的？
  Navigation_SaveLibrary(0, NULL);

  char *command = malloc(sizeof(char) * (14 + MAX_PATH + 8 + MAX_PATH + 12));
  char *user_database_path = malloc(sizeof(char) * (lib_path_len + 9));
  sprintf(user_database_path, "%s\\user.db", lib_path);
  sprintf(command, "copy /Y \"%s\" \"%s\"", user_db_dir, user_database_path);
  free(user_database_path);
  system(command);

  if (ErrorHandle(CloseDBConnection(USER))) {
    free(command);
    exit(1);
  }
  sprintf(command, "del /F %s", user_db_dir);
  system(command);

  Navigation_SaveLibrary(0, NULL);

  if (ErrorHandle(CloseDBConnection(BOOK))) {
    free(command);
    exit(1);
  }
  sprintf(command, "del /F %s", book_db_dir);
  system(command);

  if (ErrorHandle(CloseDBConnection(BORROWRECORD))) {
    free(command);
    exit(1);
  }
  sprintf(command, "del /F %s", borrowrecord_db_dir);
  system(command);

  free(command);

  ClearHistory();
  DeleteList(history_list, free);

  exit(0);  // TODO:(TO/GA)这对吗
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
      DrawUI(kWelcome, &user, NULL, msg);
      break;
    case kLendAndBorrow:  // 借还书
      PopBackHistory();
      Navigation_LendAndBorrow(msg);
      break;
    case kBookSearch: {  // 图书搜索
      char *keyword =
          malloc(sizeof(char) * strlen(history->state.book_search->keyword));
      strcpy(keyword, history->state.book_search->keyword);
      PopBackHistory();
      BookSearchDisplay(keyword, msg);
    } break;
    case kUserSearch: {  // 用户搜索（管理员）
      char *keyword =
          malloc(sizeof(char) * strlen(history->state.user_search->keyword));
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
      Navigation_UserLogInOrRegister(1, msg);
      break;
    // case kLogout:  // 用户登出
    // break;
    case kUserModify: {  // 用户信息修改
      User *new_user = malloc(sizeof(User));
      memcpy(new_user, history->state.user_modify->user, sizeof(User));
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
