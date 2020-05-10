#include "view.h"

#include <stdlib.h>
#include <string.h>

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

void Init() {
  history_list = NewList();
  DrawUI(kWelcome, &user, NULL, "");
}

static inline History *const HistoryTop() {
  return (History*)history_list->dummy_tail->pre->value;
}

static inline void HistoryPushBack(History *new_history) {
  InsertList(history_list, history_list->dummy_tail, new_history);
  while (history_list->size > HISTORY_MAX)
    EraseList(history_list, history_list->dummy_head->nxt,
              NULL);  // TODO:(TO/GA) how to free all the states?
}

static inline void ClearHistory() {
  ClearList(history_list, NULL);  // TODO:(TO/GA) how to free all the states?
}

static void LendAndBorrow_SearchCallback(char *keyword) {}

static void LendAndBorrow_ReturnCallback(ListNode *book) {}

static void BookSearch_SearchCallback(char *keyword) {}

static void BookSearch_BorrowCallback(Book *book) {}

static void UserSearch_SearchCallback(char *keyword) {}

static void UserSearch_InfoCallback(User *user) {}

static void LoginOrRegister_LoginCallback() {
  // TODO:(TO/GA) update username_len
}

static void UserModify_ConfirmCallback() {
  // TODO:(TO/GA) update username_len
  // TODO:(TO/GA) finish it
  // if (user.whoami != ADMINISTRATOR || HistoryTop()->state.user_modify->user->whoami == ADMINISTRATOR) {
  //  uint32_t sha[8];
  //  char *pwd_type = malloc();
  //  sprintf(pwd_type, "%s%s", ) Sha256Sum(sha_db, ) if (memcmp())
  //}
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
      char *msg = malloc(sizeof(char) * (25 + username_len));
      sprintf(msg, "%s: Fail to open the image", user.username);
      DrawUI(kBookDisplay, &user, HistoryTop()->state.book_display, msg);
      return;
    }
  }
  endtry;
  const int uid = HistoryTop()->state.book_display->book->uid;
  char *command =
      malloc(sizeof(char) * (25 + sizeof(image_path) + lib_path_len + 10));
  sprintf(command, "copy /Y \"%s\" \"%s\\image\\%d.jpg\"", image_path, lib_path,
          uid);

  if (system(command)) {
    char *msg = malloc(sizeof(char) * (42 + username_len + 10));
    sprintf(msg, "%s: Fail to change the book(uid = %d)'s cover", user.username,
            uid);
    DrawUI(kBookDisplay, &user, HistoryTop()->state.book_display, msg);
  } else {
    char *msg = malloc(sizeof(char) * (34 + username_len + 10));
    sprintf(msg, "%s: Change the book(uid = %d)'s cover", user.username, uid);
    DrawUI(kBookDisplay, &user, HistoryTop()->state.book_display, msg);
  }
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
  // TODO:(TO/GA) DrawImage 在没load的时候会挂的

  HistoryPushBack(new_history);

  char *msg = malloc(sizeof(char) * (25 + username_len));
  if (type) {
    sprintf(msg, "%s: Open book init page", user.username);
  } else {
    if (user.whoami == ADMINISTRATOR)
      sprintf(msg, "%s: Open book modify page", user.username);
    else
      sprintf(msg, "%s: Open book display page", user.username);
  }
  DrawUI(new_history->page, &user, new_history->state.book_display, msg);
}

static void Library_BookCallback(ListNode *book) {
  BookDisplayOrInit(book->value, 0);
}

static void Library_SwitchCallback() {}

static void Statistics_SelectCallback() {}

static inline void Navigation_LendAndBorrow(Page nav_page, User *cur_user) {
  History *new_history = malloc(sizeof(History));
  new_history->page = kLendAndBorrow;
  new_history->state.lend_and_borrow = malloc(sizeof(LendAndBorrow));
  new_history->state.lend_and_borrow->return_callback = LendAndBorrow_ReturnCallback;
  new_history->state.lend_and_borrow->search_callback = LendAndBorrow_SearchCallback;

  List *borrow_records_list = NewList();
  char *query = malloc(sizeof(char) * (31 + 10));
  sprintf(query, "user_uid=%d&book_status=BORROWED", user.uid);
  Filter(&borrowrecord_db, &borrow_records_list, query, USER); // TODO:(TO/GA) error handle
  new_history->state.lend_and_borrow->borrow_records = borrow_records_list;

  List *books = NewList();
  for (ListNode *cur_node = borrow_records_list->dummy_head;
       cur_node != borrow_records_list->dummy_tail; cur_node = cur_node->nxt) {
    Book *book = malloc(sizeof(Book));
    GetById(&book_db, &book, ((BorrowRecord*)cur_node->value)->book_uid, BORROWRECORD);
    InsertList(books, books->dummy_tail, book);
  }
  new_history->state.lend_and_borrow->books = books;

  HistoryPushBack(new_history);

  char *msg = malloc(sizeof(char) * (26 + username_len));
  sprintf(msg, "%s: Open Page LendAndBorrow", user.username);
  DrawUI(kLendAndBorrow, &user, new_history->state.lend_and_borrow, msg);
}

static inline void Navigation_BookSearch(Page nav_page, User *cur_user) {
  BookSearch_SearchCallback(NULL);
}

static inline void Navigation_UserSearch(Page nav_page, User *cur_user) {
  UserSearch_SearchCallback(NULL);
}

// type = 0 => Manual
static inline void Navigation_ManualOrAbout(Page nav_page, User *cur_user, bool type) {
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
  HistoryPushBack(new_history);

  char *msg = malloc(sizeof(char) * (19 + username_len));
  if (type)
    sprintf(msg, "%s: Open Page About", user.username);
  else
    sprintf(msg, "%s: Open Page Manual", user.username);
  DrawUI(kAbout, &user, new_history->state.manual_and_about, msg);
}

// type = 0 => LogIn
static inline void Navigation_UserLogInOrRegister(Page nav_page, User *cur_user,
                                                  bool type) {
  memset(&user, 0x00, sizeof(cur_user));

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
  HistoryPushBack(new_history);

  char *msg = malloc(sizeof(char) * 43);
  sprintf("Clear history, log out and try to %s", type ? "register" : "log in");
  DrawUI(new_history->page, &user, new_history->state.login_or_register, msg);
}

static inline void Navigation_UserLogOut(Page nav_page, User *cur_user) {
  memset(&user, 0x00, sizeof(user));
  username_len = 0;

  ClearHistory();
  History *new_history = malloc(sizeof(History));
  new_history->page = kWelcome;
  HistoryPushBack(new_history);

  char *msg = malloc(sizeof(char) * (28 + username_len));
  sprintf(msg, "%s%s", user.username, ": Clear history and log out");
  DrawUI(kWelcome, &user, new_history->state.login_or_register, msg);
}

static inline void Navigation_UserModify(Page nav_page, User *cur_user, User *show_user) {
  UserSearch_InfoCallback(&user);
}

static inline void Navigation_UserManagement(Page nav_page, User *cur_user) {
  if (user.whoami != ADMINISTRATOR) {
    char *msg = malloc(sizeof(char) * (44 + username_len));
    sprintf(msg, "%s: Don't have the permission to manage users",
            user.username);
    // 因为指针类型长度固定，所以这里的 HistoryTop()->state.book_display
    // 是随便写的，反正都一样
    DrawUI(HistoryTop()->page, &user, HistoryTop()->state.book_display,
           msg);  // TODO: (TO/GA) 可能因为什么东西没刷新而挂掉
    return;
  }
  History *new_history = malloc(sizeof(History));
  new_history->page = kUserManagement;
  new_history->state.user_management = malloc(sizeof(UserManagement));
  new_history->state.user_management->approve_callback = UserManagement_ApproveCallback;
  new_history->state.user_management->delete_callback = UserManagement_DeleteCallback;
  
  List *to_be_verified = NewList();
  Filter(&user_db, &to_be_verified, "verified=FALSE", USER);
  new_history->state.user_management->to_be_verified = to_be_verified;

  List *verified = NewList();
  Filter(&user_db, &verified, "verified=TRUE", USER);
  new_history->state.user_management->users = verified;

  HistoryPushBack(new_history);

  char *msg = malloc(sizeof(char) * (28 + username_len));
  sprintf(msg, "%s: Open user management page", user.username);
  DrawUI(kUserManagement, &user, new_history->state.user_management, msg);
}

static inline void Navigation_Library(Page nav_page, User *cur_user) {
  History *new_history = malloc(sizeof(History));
  new_history->page = kLibrary;
  new_history->state.library = malloc(sizeof(Library));
  new_history->state.library->Type = kPicture;
  new_history->state.library->sort_callback = Library_SortCallback;
  new_history->state.library->book_callback = Library_BookCallback;
  new_history->state.library->switch_callback = Library_SwitchCallback;

  List *books = NewList();
  Filter(&book_db, &books, "number_on_the_shelf!=0", BOOK);
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

  HistoryPushBack(new_history);

  char *msg = malloc(sizeof(char) * (31 + username_len));
  sprintf(msg, "%s: Open books page (image mode)", user.username);
  DrawUI(kLibrary, &user, new_history->state.library, msg);
}

// type == 0 => Open
static inline void Navigation_OpenOrInitLibrary(Page nav_page, User *cur_user, bool type) {
  if (!cur_user->uid) {
    if (type)
      DrawUI(kWelcome, &user, NULL,
             "Can't init any library. Please Login first");
    else
      DrawUI(kWelcome, &user, NULL,
             "Can't open any library. Please Login first");
    return;
  }

  try {
    SelectFolder("请选择保存图书库的文件夹", lib_path);
    except(ErrorException) {
      char *msg = malloc(sizeof(char) * (29 + username_len));
      if (type)
        sprintf(msg, "%s: Fail to init the library", user.username);
      else
        sprintf(msg, "%s: Fail to open the library", user.username);
      // 因为指针类型长度固定，所以这里的 HistoryTop()->state.book_display
      // 是随便写的，反正都一样
      DrawUI(HistoryTop()->page, &user, HistoryTop()->state.book_display,
             msg);  // TODO: (TO/GA) 有可能因为什么东西没刷新而挂掉
      return;
    }
  }
  endtry;
  lib_path_len = strlen(lib_path);

  CloseDBConnection(&book_db);
  CloseDBConnection(&user_db);
  CloseDBConnection(&borrowrecord_db);

  size_t len;
  
  if (type)  {
    len = sizeof(char) * (15 + lib_path_len);
    char *command = malloc(len);
    sprintf(command, "mkdir \"%s\\image\"", lib_path);
    if (system(command)) {
      free(command);
      char *msg = malloc(sizeof(char) * (29 + username_len));
      sprintf(msg, "%s: Fail to init the library", user.username);
      // 因为指针类型长度固定，所以这里的 HistoryTop()->state.book_display
      // 是随便写的，反正都一样
      DrawUI(HistoryTop()->page, &user, HistoryTop()->state.book_display,
             msg);  // TODO: (TO/GA) 有可能因为什么东西没刷新而挂掉
      return;
    }
    free(command);
  }

  len = sizeof(char) * (lib_path_len + strlen("file:\\\\\\book.db") + 2 + 1);
  book_db.filename = malloc(len);
  sprintf(book_db.filename, "\"%s%s%s\"", "file:\\\\", lib_path, "\\book.db");
  OpenDBConnection(&book_db);  // TODO:(TO/GA) 异常处理

  user_db.filename = malloc(len);
  sprintf(user_db.filename, "\"%s%s%s\"", "file:\\\\", lib_path, "\\user.db");
  OpenDBConnection(&user_db);

  len += sizeof(char) *
         (strlen("file:\\\\\\borrowrecord.db") - strlen("file:\\\\\\book.db"));
  borrowrecord_db.filename = malloc(len);
  sprintf(borrowrecord_db.filename, "\"%s%s%s\"", "file:\\\\", lib_path,
          "\\borrowrecord.db");
  OpenDBConnection(&borrowrecord_db);

  ClearHistory();
  History *new_history = malloc(sizeof(History));
  new_history->page = kWelcome;
  HistoryPushBack(new_history);

  len += sizeof(char) * (strlen(": Clear history and init library from ") -
                         strlen("file:\\\\\\borrowrecord.db") + username_len);
  char *msg = malloc(len);
  if (type)
    sprintf(msg, "%s%s%s", user.username,
            ": Clear history and init library from ", lib_path);
  else
    sprintf(msg, "%s%s%s", user.username,
            ": Clear history and open library from ", lib_path);
  DrawUI(kWelcome, &user, NULL, msg);
}

static inline void Navigation_BookInit(Page nav_page, User *cur_user) {
  Book *book = malloc(sizeof(Book));
  book->uid = GetNextPK(&book_db, BOOK);
  BookDisplayOrInit(book, 1);
}

static inline void Navigation_Statistics(Page nav_page, User *cur_user) {

}

static inline void Navigation_Return(Page nav_page, User *cur_user) {

}

void NavigationCallback(Page nav_page, User *cur_user) { // TODO:(TO/GA) cur_user 这个参数好像没用？
  switch (nav_page) {
    case kLendAndBorrow:  // 借还书
      Navigation_LendAndBorrow(nav_page, cur_user);  // TODO:(TO/GA) nav_page 这个参数好像没用？
      break;
    case kBookSearch:  // 图书搜索
      Navigation_BookSearch(nav_page, cur_user);
      break;
    case kUserSearch:  // 用户搜索（管理员）
      Navigation_UserSearch(nav_page, cur_user);
      break;
    case kManual:  // 帮助
      Navigation_ManualOrAbout(nav_page, cur_user, 0);
      break;
    case kAbout:  // 关于
      Navigation_ManualOrAbout(nav_page, cur_user, 1);
      break;
    case kUserRegister:  // 用户注册
      Navigation_UserLogInOrRegister(nav_page, cur_user, 1);
      break;
    case kUserLogIn:  // 用户登陆
      Navigation_UserLogInOrRegister(nav_page, cur_user, 0);
      break;
    case kLogout:  // 用户登出
      Navigation_UserLogOut(nav_page, cur_user);
      break;
    case kUserModify:  // 用户信息修改
      Navigation_UserModify(nav_page, cur_user, &user);
      break;
    case kUserManagement:  // 用户删除/审核（管理员）
      Navigation_UserManagement(nav_page, cur_user);
      break;
    case kLibrary:  // 图书库显示
      Navigation_Library(nav_page, cur_user);
      break;
    case kInitLibrary:  // 图书库新建
      Navigation_OpenOrInitLibrary(nav_page, cur_user, 1);
      break;
    case kOpenLibrary:  // 图书库打开
      Navigation_OpenOrInitLibrary(nav_page, cur_user, 0);
      break;
    // case kBookDisplay:  // 图书显示
      // Navigation_BookDisplay(nav_page, cur_user);
      // break;
    case kBookInit:  // 图书新增
      Navigation_BookInit(nav_page, cur_user);
      break;
    // case kBookModify:  // 图书修改/删除
      // Navigation_BookModify(nav_page, cur_user);
      // break;
    // case kBorrowDisplay:  // 借还书统计（管理员）
      // Navigation_BorrowDisplay(nav_page, cur_user);
      // break;
    case kStatistics:  // 统计
      Navigation_Statistics(nav_page, cur_user);
      break;
    case kReturn:  // 回到上一个界面
      Navigation_Return(nav_page, cur_user);
      break;
    default:
      Error("Unknown nav_page");
  }
}
