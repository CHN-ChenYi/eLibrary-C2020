#include "view.h"

#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "gui.h"
#include "list.h"
#include "basictype.h"
#include "exception.h"
#include "model.h"
#include "hash.h"

void DrawUI(Page cur_page, User *cur_user, void *info, char *terminal) {}

typedef struct History {
  Page page;
  State state;
} History;

static List *history_list;
static User user;
static char lib_path[MAX_PATH + 1];
static size_t lib_path_len, username_len;
static DB book_db, user_db, borrowrecord_db;
static FILE *log_file;

inline void Log(char *const msg) {
  time_t cur_time = time(0);
  char *time = asctime(localtime(&cur_time));
  size_t len = strlen(time);
  while (len && (time[len - 1] == '\r' || time[len - 1] == '\n'))
    time[--len] = '\0';
  // As DST is not always one hour, calculating loacl time zone is expensive
  fprintf(log_file, "[%s Local] %s\n", time, msg);
}

void Init() {
  InitConsole();
  InitGraphics();
  history_list = NewList();
  fopen_s(&log_file, ".\\eLibrary.log", "a+");
  Log("[Info] Start");
  DrawUI(kWelcome, &user, NULL, "");
}

static void FreeHistory(void *const history_) {
  History *const history = history_;
  switch (history->page) {
    case kWelcome: // 欢迎界面
      break;
    case kLendAndBorrow:  // 借还书
      DeleteList(history->state.lend_and_borrow->books, NULL);
      DeleteList(history->state.lend_and_borrow->borrow_records, NULL);
      break;
    case kBookSearch:  // 图书搜索
      free(history->state.book_search->keyword);
      DeleteList(history->state.book_search->book_result, NULL);
      break;
    case kUserSearch:  // 用户搜索（管理员）
      free(history->state.user_search->keyword);
      DeleteList(history->state.user_search->user_result, NULL);
      break;
    case kManual:  // 帮助
    case kAbout:   // 关于
      free(history->state.manual_and_about->title);
      free(history->state.manual_and_about->content);
      break;
    case kUserRegister:  // 用户注册
    case kUserLogIn:     // 用户登陆
      break;
    // case kLogout:  // 用户登出
      // break;
    case kUserModify:  // 用户信息修改
      DeleteList(history->state.user_modify->books, NULL);
      break;
    case kUserManagement:  // 用户删除/审核（管理员）
      DeleteList(history->state.user_management->to_be_verified, NULL);
      DeleteList(history->state.user_management->users, NULL);
      break;
    case kLibrary:  // 图书库显示
      DeleteList(history->state.library->books, NULL);
      DeleteList(history->state.library->book_covers, free);
      break;
    // case kInitLibrary:  // 图书库新建
      // break;
    // case kOpenLibrary:  // 图书库打开
      // break;
    case kBookDisplay:  // 图书显示
      free(history->state.borrow_display->book_name); // TODO:(TO/GA) 这玩意儿有可能和之前的东西是共享的...要么统一深拷贝？
      break;
    case kBookInit:  // 图书新增
    case kBookModify:  // 图书修改/删除
      free(history->state.book_display->book);
      break;
    // Navigation_BookModify(nav_page, cur_user);
    // break;
    // case kBorrowDisplay:  // 借还书统计（管理员）
    // Navigation_BorrowDisplay(nav_page, cur_user);
    // break;
    case kStatistics:  // 统计
      DeleteList(history->state.statistics->catalogs, free);
      DeleteList(history->state.statistics->borrow_record, NULL);
      break;
    // case kReturn:  // 回到上一个界面
      // break;
    default:
      Log("[Debug] Unknown page in FreeHistory");
      Error("Unknown nav_page");
  }
  free(history);
}

static inline History *const TopHistory() {
  return (History*)history_list->dummy_tail->pre->value;
}

static inline void PushBackHistory(History *const new_history) {
  InsertList(history_list, history_list->dummy_tail, new_history);
  while (history_list->size > HISTORY_MAX)
    EraseList(history_list, history_list->dummy_head->nxt,
              FreeHistory);
}

static inline void ClearHistory() {
  ClearList(history_list,
            FreeHistory);
}

static inline void ReturnHistory(ListNode *go_back_to, const char *const msg);

static void BookSearch_BorrowCallback(
    Book *book);

static void BookSearch_SearchCallback(char *keyword) {
  History *new_history = malloc(sizeof(History));
  new_history->page = kBookSearch;
  new_history->state.book_search = malloc(sizeof(BookSearch));
  new_history->state.book_search->keyword = keyword;
  new_history->state.book_search->borrow_callback = BookSearch_BorrowCallback;
  new_history->state.book_search->search_callback = BookSearch_SearchCallback;

  List *results = NewList();
  Filter(&book_db, results, keyword, BOOK);  // TODO:(TO/GA) Error Handle
  new_history->state.book_search->book_result = results;

  PushBackHistory(new_history);

  char *msg = malloc(sizeof(char) * (23 + username_len + strlen(keyword)));
  sprintf(msg, "[Info] [%s] Book search %s", user.username, keyword);
  Log(msg);
  DrawUI(kBookSearch, &user, new_history->state.book_search, msg);
}

static void BookSearch_BorrowCallback(Book *book) {
  if (!book->number_on_the_shelf) {
    char *msg = malloc(sizeof(char) * (38 + username_len + strlen(book->title)));
    sprintf(msg, "[Error] [%s] There's no [%s] on the shelf", user.username, book->title);
    ReturnHistory(history_list->dummy_tail->pre, msg);
    return;
  }
  book->number_on_the_shelf--;
  Update(&book_db, book, book->uid, BOOK);

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
  new_record.uid = GetNextPK(&borrowrecord_db, BORROWRECORD);
  strcpy(new_record.user_name, user.username);
  new_record.user_uid = user.uid;
  Create(&borrowrecord_db, &new_record, BORROWRECORD);

  char *msg = malloc(sizeof(char) * (25 + username_len + strlen(book->title)));
  sprintf(msg, "[Info] [%s] Borrow book [%s]", user.username, book->title);
  ReturnHistory(history_list->dummy_tail->pre, msg);
}


static void LendAndBorrow_SearchCallback(char *keyword) {
  BookSearch_SearchCallback(keyword);
}

static void LendAndBorrow_ReturnCallback(ListNode *book,
                                         ListNode *borrow_record) {
  Book *returned_book = (Book *)book->value;
  BorrowRecord *returned_borrow_record = (BorrowRecord *)borrow_record->value;

  returned_book->number_on_the_shelf++;
  Update(&book_db, returned_book, returned_book->uid, BOOK);

  char *msg = malloc(sizeof(char) * (49 + strlen(returned_book->title) + 8));
  sprintf(msg, "[Info] [%s] Return book [%s], expected return date[%s]", user.username, returned_book->title, returned_borrow_record->returned_date);

  returned_borrow_record->book_status = RETURNED;
  time_t now_time_t = time(0);
  struct tm *now_tm = localtime(&now_time_t);
  sprintf(returned_borrow_record->returned_date, "%04d%02d%02d",
          now_tm->tm_year + 1900, now_tm->tm_mon + 1, now_tm->tm_mday);
  Update(&borrowrecord_db, returned_borrow_record, returned_borrow_record->uid, BORROWRECORD);

  ReturnHistory(history_list->dummy_tail->pre, msg);
}

static void UserModify_ConfirmCallback() {
  // TODO:(TO/GA) update username_len
  // TODO:(TO/GA) finish it
  // if (user.whoami != ADMINISTRATOR ||
  // TopHistory()->state.user_modify->user->whoami == ADMINISTRATOR) {
  //  uint32_t sha[8];
  //  char *pwd_type = malloc();
  //  sprintf(pwd_type, "%s%s", ) Sha256Sum(sha_db, ) if (memcmp())
  //}
}

static void UserSearch_InfoCallback(User *user);

static void UserSearch_SearchCallback(char *keyword) {
  if (user.whoami != ADMINISTRATOR) {
    char *msg = malloc(sizeof(char) * (49 + username_len));
    sprintf(msg, "[Error] [%s] Permission denied. Can't search users",
            user.username);
    ReturnHistory(history_list->dummy_tail->pre, msg);
    return;
  }
  History *new_history = malloc(sizeof(History));
  new_history->page = kBookSearch;
  new_history->state.user_search = malloc(sizeof(UserSearch));
  new_history->state.user_search->keyword = keyword;
  new_history->state.user_search->info_callback = UserSearch_InfoCallback;
  new_history->state.user_search->search_callback = UserSearch_SearchCallback;

  List *results = NewList();
  Filter(&user_db, results, keyword, USER);  // TODO:(TO/GA) Error Handle
  new_history->state.user_search->user_result = results;

  PushBackHistory(new_history);

  char *msg = malloc(sizeof(char) * (23 + username_len + strlen(keyword)));
  sprintf(msg, "[Info] [%s] User search %s", user.username, keyword);
  Log(msg);
  DrawUI(kUserSearch, &user, new_history->state.user_search, msg);
}

static void UserSearch_InfoCallback(User *show_user) {
  History *new_history = malloc(sizeof(History));
  new_history->page = kUserModify;
  new_history->state.user_modify = malloc(sizeof(UserModify));
  new_history->state.user_modify->confirm_callback = UserModify_ConfirmCallback;
  memcpy(new_history->state.user_modify->user, show_user, sizeof(User));

  List *borrow_record = NewList(), *books = NewList();
  char *query = malloc(sizeof(char) * (10 + 10));
  sprintf(query, "user_uid=%d", show_user->uid);
  Filter(&borrowrecord_db, borrow_record, query, BORROWRECORD);
  free(query);
  for (ListNode *cur_node = borrow_record->dummy_head->nxt; cur_node != borrow_record->dummy_tail; cur_node = cur_node->nxt) {
    Book *new_book = malloc(sizeof(BOOK));
    GetById(&book_db, new_book, ((BorrowRecord*)cur_node->value)->book_uid, BOOK);
    InsertList(books, books->dummy_tail->pre, new_book);
  }
  new_history->state.user_modify->books = books;
  
  PushBackHistory(new_history);

  char *msg = malloc(sizeof(char) * (30 + username_len + strlen(show_user->username)));
  sprintf(msg, "[Info] [%s] Show/Modify user [%s]", user.username, show_user->username);
  Log(msg);
  DrawUI(kUserModify, &user, new_history->state.user_modify, msg);
}

static void LoginOrRegister_LoginCallback() {
  // TODO:(TO/GA) update username_len
}

static void UserManagement_ApproveCallback(ListNode *user, bool approve) {}

static void UserManagement_DeleteCallback(ListNode *user) {}

static void BookDisplay_AdminCallback() {}

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
  sprintf(msg, "[Info] [%s] Change the book(uid = %d)'s cover", user.username, uid);
  Log(msg);
  DrawUI(kBookDisplay, &user, TopHistory()->state.book_display, msg);
  free(command);
}

static void BookDisplay_ConfirmCallback() {}

static void BookDisplay_DeleteCallback() {}

static void BookDisplay_BorrowCallback() {}

static void Library_SortCallback(SortKeyword sort_keyword) {}

// type = 0 => Display
static void BookDisplayOrInit(Book *book, bool type) {
  History *new_history = malloc(sizeof(History));
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
  new_history->state.book_display->book = book;

  if (!type) {
    char *image_path = malloc(sizeof(char) * (14 + lib_path_len + 10));
    sprintf(image_path, "\"%s\\image\\%d.jpg\"", lib_path, book->uid);
    loadImage(image_path, &new_history->state.book_display->book_cover);
  }

  PushBackHistory(new_history);

  char *msg = malloc(sizeof(char) * (33 + username_len));
  if (type) {
    sprintf(msg, "[Info] [%s] Open book init page", user.username);
  } else {
    if (user.whoami == ADMINISTRATOR)
      sprintf(msg, "[Info] [%s] Open book modify page", user.username);
    else
      sprintf(msg, "[Info] [%s] Open book display page", user.username);
  }
  Log(msg);
  DrawUI(new_history->page, &user, new_history->state.book_display, msg);
}

static void Library_BookCallback(ListNode *book) {
  BookDisplayOrInit(book->value, 0);
}

static void Library_SwitchCallback() {}

static void Statistics_SelectCallback(ListNode *catalog) {}

static inline void Navigation_LendAndBorrow() {
  History *new_history = malloc(sizeof(History));
  new_history->page = kLendAndBorrow;
  new_history->state.lend_and_borrow = malloc(sizeof(LendAndBorrow));
  new_history->state.lend_and_borrow->return_callback = LendAndBorrow_ReturnCallback;
  new_history->state.lend_and_borrow->search_callback = LendAndBorrow_SearchCallback;

  List *borrow_records_list = NewList();
  char *query = malloc(sizeof(char) * (31 + 10));
  sprintf(query, "user_uid=%d&book_status=BORROWED", user.uid);
  Filter(&borrowrecord_db, borrow_records_list, query, USER); // TODO:(TO/GA) error handle
  free(query);
  new_history->state.lend_and_borrow->borrow_records = borrow_records_list;

  List *books = NewList();
  for (ListNode *cur_node = borrow_records_list->dummy_head;
       cur_node != borrow_records_list->dummy_tail; cur_node = cur_node->nxt) {
    Book *book = malloc(sizeof(Book));
    GetById(&book_db, book, ((BorrowRecord*)cur_node->value)->book_uid, BORROWRECORD);
    InsertList(books, books->dummy_tail, book);
  }
  new_history->state.lend_and_borrow->books = books;

  PushBackHistory(new_history);

  char *msg = malloc(sizeof(char) * (34 + username_len));
  sprintf(msg, "[Info] [%s] Open Page LendAndBorrow", user.username);
  Log(msg);
  DrawUI(kLendAndBorrow, &user, new_history->state.lend_and_borrow, msg);
}

static inline void Navigation_BookSearch() {
  BookSearch_SearchCallback(NULL);
}

static inline void Navigation_UserSearch() {
  UserSearch_SearchCallback(NULL);
}

// type = 0 => Manual
static inline void Navigation_ManualOrAbout(bool type) {
  History *new_history = malloc(sizeof(History));
  new_history->state.manual_and_about = malloc(sizeof(ManualAndAbout));
  // TODO:(TO/GA) finish it
  if (type) {
    new_history->page = kAbout;
    new_history->state.manual_and_about->title = "About ...";
    new_history->state.manual_and_about->content = "233 ...";
  } else {
    new_history->page = kManual;
    new_history->state.manual_and_about->title = "Manual ...";
    new_history->state.manual_and_about->content = "233 ...";
  }
  PushBackHistory(new_history);

  char *msg = malloc(sizeof(char) * (27 + username_len));
  if (type)
    sprintf(msg, "[Info] [%s] Open Page About", user.username);
  else
    sprintf(msg, "[Info] [%s] Open Page Manual", user.username);
  Log(msg);
  DrawUI(kAbout, &user, new_history->state.manual_and_about, msg);
}

// type = 0 => LogIn
static inline void Navigation_UserLogInOrRegister(bool type) {
  memset(&user, 0x00, sizeof(user));

  ClearHistory();
  History *new_history = malloc(sizeof(History));
  if (type)
    new_history->page = kUserRegister;
  else
    new_history->page = kUserLogIn;
  new_history->state.login_or_register = malloc(sizeof(LoginOrRegister));
  new_history->state.login_or_register->user = &user;
  new_history->state.login_or_register->login_callback =
      LoginOrRegister_LoginCallback;
  PushBackHistory(new_history);

  char *msg = malloc(sizeof(char) * 38);
  sprintf(msg, "[Info] Clear history, try to %s", type ? "register" : "log in");
  Log(msg);
  DrawUI(new_history->page, &user, new_history->state.login_or_register, msg);
}

static inline void Navigation_UserLogOut() {
  memset(&user, 0x00, sizeof(user));
  username_len = 0;

  ClearHistory();
  History *new_history = malloc(sizeof(History));
  new_history->page = kWelcome;
  PushBackHistory(new_history);

  char *msg = malloc(sizeof(char) * (36 + username_len));
  sprintf(msg, "[Info] [%s] Clear history and log out", user.username);
  Log(msg);
  DrawUI(kWelcome, &user, new_history->state.login_or_register, msg);
}

static inline void Navigation_UserModify() {
  UserSearch_InfoCallback(&user);
}

static inline void Navigation_UserManagement() {
  if (user.whoami != ADMINISTRATOR) {
    char *msg = malloc(sizeof(char) * (49 + username_len));
    sprintf(msg, "[Error] [%s] Permission denied. Can't manage users",
            user.username);
    ReturnHistory(history_list->dummy_tail->pre, msg);
    return;
  }
  History *new_history = malloc(sizeof(History));
  new_history->page = kUserManagement;
  new_history->state.user_management = malloc(sizeof(UserManagement));
  new_history->state.user_management->approve_callback = UserManagement_ApproveCallback;
  new_history->state.user_management->delete_callback = UserManagement_DeleteCallback;
  
  List *to_be_verified = NewList();
  Filter(&user_db, to_be_verified, "verified=FALSE", USER);
  new_history->state.user_management->to_be_verified = to_be_verified;

  List *verified = NewList();
  Filter(&user_db, verified, "verified=TRUE", USER);
  new_history->state.user_management->users = verified;

  PushBackHistory(new_history);

  char *msg = malloc(sizeof(char) * (36 + username_len));
  sprintf(msg, "[Info] [%s] Open Page User Management", user.username);
  Log(msg);
  DrawUI(kUserManagement, &user, new_history->state.user_management, msg);
}

static inline void Navigation_Library() {
  History *new_history = malloc(sizeof(History));
  new_history->page = kLibrary;
  new_history->state.library = malloc(sizeof(Library));
  new_history->state.library->Type = kPicture;
  new_history->state.library->sort_callback = Library_SortCallback;
  new_history->state.library->book_callback = Library_BookCallback;
  new_history->state.library->switch_callback = Library_SwitchCallback;

  List *books = NewList();
  Filter(&book_db, books, "", BOOK);
  new_history->state.library->books = books;

  List *book_covers = NewList();
  const size_t image_path_len = 11 + lib_path_len;
  char *image_path = malloc(sizeof(char) * (image_path_len + 14));
  sprintf(image_path, "\"%s\\image\\", lib_path);
  for (ListNode *cur_node = books->dummy_head->nxt; cur_node != books->dummy_tail; cur_node = cur_node->nxt) {
    LibImage *image = malloc(sizeof(LibImage));
    sprintf(image_path + image_path_len, "%d.jpg", ((Book*)cur_node->value)->uid);
    loadImage(image_path, image);
    InsertList(book_covers, book_covers->dummy_tail, image);
  }
  new_history->state.library->book_covers = book_covers;

  PushBackHistory(new_history);

  char *msg = malloc(sizeof(char) * (41 + username_len));
  sprintf(msg, "[Info] [%s] Open Page Library (image mode)", user.username);
  Log(msg);
  DrawUI(kLibrary, &user, new_history->state.library, msg);
}

// type == 0 => Open
static inline void Navigation_OpenOrInitLibrary(bool type) {
  if (!user.uid) {
    char *msg = malloc(sizeof(char) * 70);
    if (type)
      sprintf(msg, "[Error] Permission denied. Can't init any library. Please Login first");
    else
      sprintf(msg, "[Error] Permission denied. Can't open any library. Please Login first");
    Log(msg);
    DrawUI(kWelcome, &user, NULL, msg);
    return;
  }

  try {
    SelectFolder("请选择保存图书库的文件夹", lib_path);
    except(ErrorException) {
      char *msg = malloc(sizeof(char) * (56 + username_len));
      if (type)
        sprintf(msg, "[Error] [%s] Fail to init the library. Path doesn't exist", user.username);
      else
        sprintf(msg, "[Error] [%s] Fail to open the library. Path doesn't exist", user.username);
      ReturnHistory(history_list->dummy_tail->pre, msg);
      return;
    }
  }
  endtry;
  lib_path_len = strlen(lib_path);

  CloseDBConnection(&book_db, BOOK);
  CloseDBConnection(&user_db, USER);
  CloseDBConnection(&borrowrecord_db, BORROWRECORD);

  size_t len;
  
  if (type)  {
    len = sizeof(char) * (15 + lib_path_len);
    char *command = malloc(len);
    sprintf(command, "mkdir \"%s\\image\"", lib_path);
    if (system(command)) {
      free(command);
      char *msg = malloc(sizeof(char) * (63 + username_len));
      sprintf(msg, "[Error] [%s] Fail to init the library. Can't create image folder", user.username);
      ReturnHistory(history_list->dummy_tail->pre, msg);
      return;
    }
    free(command);
  }

  len = sizeof(char) * (lib_path_len + strlen("file:\\\\\\book.db") + 2 + 1);
  book_db.filename = malloc(len);
  sprintf(book_db.filename, "\"%s%s%s\"", "file:\\\\", lib_path, "\\book.db");
  OpenDBConnection(&book_db, BOOK);  // TODO:(TO/GA) 异常处理

  user_db.filename = malloc(len);
  sprintf(user_db.filename, "\"%s%s%s\"", "file:\\\\", lib_path, "\\user.db");
  OpenDBConnection(&user_db, USER);

  len += sizeof(char) *
         (strlen("file:\\\\\\borrowrecord.db") - strlen("file:\\\\\\book.db"));
  borrowrecord_db.filename = malloc(len);
  sprintf(borrowrecord_db.filename, "\"%s%s%s\"", "file:\\\\", lib_path,
          "\\borrowrecord.db");
  OpenDBConnection(&borrowrecord_db, BORROWRECORD);

  ClearHistory();
  History *new_history = malloc(sizeof(History));
  new_history->page = kWelcome;
  PushBackHistory(new_history);

  len = sizeof(char) * (47 + lib_path_len + username_len);
  char *msg = malloc(len);
  if (type)
    sprintf(msg, "[Info] [%s] Clear history and init library from %s",
            user.username, lib_path);
  else
    sprintf(msg, "[Info] [%s] Clear history and open library from %s",
            user.username, lib_path);
  Log(msg);
  DrawUI(kWelcome, &user, NULL, msg);
}

static inline void Navigation_BookInit() {
  Book *book = malloc(sizeof(Book));
  book->uid = GetNextPK(&book_db, BOOK);
  BookDisplayOrInit(book, 1);
}

static bool StrLess(const void *const lhs, const void *rhs) {
  return strcmp(lhs, rhs) <= 0;
}
static bool StrSame(const void *const lhs, const void *rhs) {
  return strcmp(lhs, rhs) == 0;
}
static void StrFree(void *p) {
  free(p);
}

static inline void Navigation_Statistics() {
  History *new_history = malloc(sizeof(History));
  new_history->page = kStatistics;
  new_history->state.statistics = malloc(sizeof(Statistics));
  new_history->state.statistics->select_callback = Statistics_SelectCallback;

  List *book = NewList(), *category = NewList();
  Filter(&book_db, book, "", BOOK);
  for (ListNode *cur_node = book->dummy_head->nxt; cur_node != book->dummy_tail; cur_node = cur_node->nxt) {
    char *str = malloc(sizeof(char) * (strlen(((Book*)cur_node->value)->category) + 1));
    strcpy(str, ((Book *)cur_node->value)->category);
    InsertList(category, category->dummy_tail, str);
  }
  DeleteList(book, NULL);
  SortList(category, StrLess);
  UniqueList(category, StrSame, StrFree);
  new_history->state.statistics->catalogs = category;

  List *borrow_record = NewList();
  Filter(&borrowrecord_db, borrow_record, "", BORROWRECORD);
  new_history->state.statistics->borrow_record = borrow_record;

  PushBackHistory(new_history);

  char *msg = malloc(sizeof(char) * (31 + username_len));
  sprintf(msg, "[Info] [%s] Open Page Statistics", user.username);
  Log(msg);
  DrawUI(kStatistics, &user, new_history->state.statistics, msg);
}

static inline void Navigation_Return() {
  if (history_list->size < 2) {
    char *msg = malloc(sizeof(char) * (44 + username_len));
    sprintf(msg, "[Error] [%s] There's no history to go back to", user.username);
    ReturnHistory(history_list->dummy_tail->pre, msg);
  }else {
    char *msg = malloc(sizeof(char) * (18 + username_len));
    sprintf(msg, "[Info] [%s] Go back", user.username);
    ReturnHistory(history_list->dummy_tail->pre->pre, msg);
  }
}

void NavigationCallback(Page nav_page, User *cur_user) { // TODO:(TO/GA) cur_user 这个参数好像没用？
  switch (nav_page) {
    // case kWelcome: // 欢迎界面
      // break;
    case kLendAndBorrow:  // 借还书
      Navigation_LendAndBorrow();
      break;
    case kBookSearch:  // 图书搜索
      Navigation_BookSearch();
      break;
    case kUserSearch:  // 用户搜索（管理员）
      Navigation_UserSearch();
      break;
    case kManual:  // 帮助
      Navigation_ManualOrAbout(0);
      break;
    case kAbout:  // 关于
      Navigation_ManualOrAbout(1);
      break;
    case kUserRegister:  // 用户注册
      Navigation_UserLogInOrRegister(1);
      break;
    case kUserLogIn:  // 用户登陆
      Navigation_UserLogInOrRegister(0);
      break;
    case kLogout:  // 用户登出
      Navigation_UserLogOut();
      break;
    case kUserModify:  // 用户信息修改
      Navigation_UserModify();
      break;
    case kUserManagement:  // 用户删除/审核（管理员）
      Navigation_UserManagement();
      break;
    case kLibrary:  // 图书库显示
      Navigation_Library();
      break;
    case kInitLibrary:  // 图书库新建
      Navigation_OpenOrInitLibrary(1);
      break;
    case kOpenLibrary:  // 图书库打开
      Navigation_OpenOrInitLibrary(0);
      break;
    // case kBookDisplay:  // 图书显示
      // break;
    case kBookInit:  // 图书新增
      Navigation_BookInit();
      break;
    // case kBookModify:  // 图书修改/删除
      // break;
    // case kBorrowDisplay:  // 借还书统计（管理员）
      // break;
    case kStatistics:  // 统计
      Navigation_Statistics();
      break;
    case kReturn:  // 回到上一个界面
      Navigation_Return();
      break;
    default:
      Log("[Debug] Unknown nav_page in NavigationCallback");
      Error("Unknown nav_page");
  }
}

static inline void ReturnHistory(ListNode *go_back_to, const char *const msg) {
  while (history_list->dummy_tail->pre != go_back_to)
    EraseList(history_list, history_list->dummy_tail->pre,
              FreeHistory);
  // TODO:(TO/GA) finish it
}
