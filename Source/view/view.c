#include "view.h"

#include <Windows.h>
#include <io.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "basictype.h"
#include "exception.h"
#include "extgraph.h"
#include "graphics.h"
#include "gui.h"
#include "hash.h"
#include "history.h"
#include "list.h"
#include "model.h"
#include "utility.h"

extern List *history_list;
extern bool db_open;

User user;
size_t id_len;

static char lib_dir[MAX_PATH + 1], image_dir[MAX_PATH + 1];
static size_t lib_dir_len, image_dir_len;
static char book_db_dir[MAX_PATH + 1], user_db_dir[MAX_PATH + 1],
    borrowrecord_db_dir[MAX_PATH + 1];
static LibImage edit_cover, unknown_cover;
static void BookSearch_BorrowCallback(Book *book);
static void BookSearch_BookCallback(Book *book);
static void BookSearch_SearchCallback(char *keyword);
static void BookSearch_TurnPage(bool direction);
static void LendAndBorrow_SearchCallback(char *keyword);
static void LendAndBorrow_ReturnCallback(ListNode *book,
                                         ListNode *borrow_record);
static void LendAndBorrow_TurnPage(bool direction);
static void UserModify_ConfirmCallback();
static void UserModify_TurnPage(bool direction);
static void UserSearch_InfoCallback(User *user);
static void UserSearch_SearchCallback(char *keyword);
static void UserSearch_TurnPage(bool direction);
static void LoginOrRegister_LoginCallback();
static void UserManagement_ApproveCallback(ListNode *user_node, bool approve);
static void UserManagement_DeleteCallback(ListNode *user_node);
static void UserManagement_TurnPage(bool direction, bool type);
static void UserManagement_InfoCallback(User *user);
static void UserManagement_SortCallback(SortKeyword sort_keyword);
static void BookDisplay_AdminCallback();
static void BookDisplay_CoverCallback();
static void BookDisplay_ConfirmCallback();
static void BookDisplay_DeleteCallback();
static void BookDisplay_BorrowCallback();
static void BookDisplay_CopyPasteCallback();
static void BorrowDisplay_TurnPage(bool direction);
static void Library_BookCallback(ListNode *book);
static void Library_SortCallback(SortKeyword sort_keyword);
static void Library_SwitchCallback();
static void Library_TurnPage(bool direction);
static void Statistics_SelectCallback(ListNode *catalog);
static void Statistics_TurnPage(bool direction, bool type);

void InitView() {
  InitHistory();
  InitUtility();

  // load resource
  try {
    loadImage(".\\Resource\\edit_cover.jpg", &edit_cover);
    loadImage(".\\Resource\\unknown_cover.jpg", &unknown_cover);
    except(ErrorException) {
      Log("[Debug] Fail to load resources");
      exit(1);
    }
  }
  endtry;

  History *const new_history = malloc(sizeof(History));
  new_history->page = kWelcome;
  PushBackHistory(new_history);

  // set up welcome page
  char *msg = malloc(sizeof(char) * 13);
  sprintf(msg, "[Info] Start");
  Log(msg);
  DrawUI(kWelcome, &user, NULL, msg);
}

void BookSearch_BorrowCallback(Book *book) {
  // 减少在架数量
  if (!book->number_on_the_shelf) {
    char *msg = malloc(sizeof(char) * (38 + id_len + strlen(book->id)));
    sprintf(msg, "[Error] [%s] There's no [%s] on the shelf", user.id,
            book->id);
    ReturnHistory(history_list->dummy_tail->pre, msg);
    return;
  }
  book->number_on_the_shelf--;
  if (ErrorHandle(Update(book, book->uid, BOOK), 0)) return;

  // 新建借阅记录
  BorrowRecord new_record;
  strcpy(new_record.book_id, book->id);
  new_record.book_status = BORROWED;
  new_record.book_uid = book->uid;
  strcpy(new_record.borrowed_date, GetTime(time(NULL)));
  strcpy(new_record.returned_date,
         GetTime(time(NULL) + (time_t)86400 * book->available_borrowed_days));
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

void BookSearch_BookCallback(Book *book) {
  Navigation_BookDisplayOrInit(book, 0, NULL);
}

void BookSearchDisplay(char *keyword, char *msg) {
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

void BookSearch_SearchCallback(char *keyword) {
  char *new_keyword = malloc(sizeof(char) * (strlen(keyword) + 1));
  strcpy(new_keyword, keyword);
  BookSearchDisplay(new_keyword, NULL);
}

void BookSearch_TurnPage(bool direction) {
  BookSearch *state = TopHistory()->state.book_search;
  char *msg = MoveInList(&state->book_result_start, state->book_result,
                         kBookSearchMax, direction, "Result", "Book Search");
  DrawUI(kBookSearch, &user, state, msg);
}

void LendAndBorrow_SearchCallback(char *keyword) {
  // 由于 BookSearch 的 Callback 里面已经有了 keyword
  // 的深拷贝，所以这里浅拷贝就可以了
  BookSearch_SearchCallback(keyword);
}

void LendAndBorrow_ReturnCallback(ListNode *book, ListNode *borrow_record) {
  Book *returned_book = (Book *)book->value;
  BorrowRecord *returned_borrow_record = (BorrowRecord *)borrow_record->value;

  // 增加要还的书的在架数
  returned_book->number_on_the_shelf++;
  if (ErrorHandle(Update(returned_book, returned_book->uid, BOOK), 0)) return;

  char *msg = malloc(sizeof(char) * (49 + strlen(returned_book->id) + 16));
  sprintf(msg, "[Info] [%s] Return book [%s], expected return date[%s]",
          user.id, returned_book->id, returned_borrow_record->returned_date);

  // 将借还记录的状态改成已还并更新还书的时间
  returned_borrow_record->book_status = RETURNED;
  strcpy(returned_borrow_record->returned_date, GetTime(time(NULL)));
  if (ErrorHandle(Update(returned_borrow_record, returned_borrow_record->uid,
                         BORROWRECORD),
                  0)) {
    free(msg);
    return;
  }

  ReturnHistory(history_list->dummy_tail->pre, msg);
}

void LendAndBorrow_TurnPage(bool direction) {
  LendAndBorrow *state = TopHistory()->state.lend_and_borrow;
  char *msg =
      MoveInList(&state->borrow_records_start, state->borrow_records,
                 kLendAndBorrowMax, direction, "Record", "Lend And Borrow");
  DrawUI(kLendAndBorrow, &user, state, msg);
}

void UserModify_ConfirmCallback() {
  User *modified_user = TopHistory()->state.user_modify->user;

  // 如果当前用户和待修改用户权限等级相同，则检查密码是否正确
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

  // 如果设置了新的密码
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

  // 用户号不能为空
  if (modified_user->id[0] == '\0') {
    char *msg = malloc(sizeof(char) * (51 + id_len + 10));
    sprintf(msg, "[Error] [%s] User [uid = %d]'s id can't be blank", user.id,
            modified_user->uid);
    ReturnHistory(history_list->dummy_tail->pre, msg);
    return;
  }

  // 用户号不能重复
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

  // 更新数据库
  if (ErrorHandle(Update(modified_user, modified_user->uid, USER), 0)) return;
  if (modified_user->uid == user.uid) {
    memcpy(&user, modified_user, sizeof(User));
    id_len = strlen(user.id);
  }

  char *msg = malloc(sizeof(char) * (32 + id_len + strlen(modified_user->id)));
  sprintf(msg, "[Info] [%s] Modify user [%s]'s info", user.id,
          modified_user->id);
  ReturnHistory(history_list->dummy_tail->pre, msg);
}

void UserModify_TurnPage(bool direction) {
  UserModify *state = TopHistory()->state.user_modify;
  char *msg = MoveInList(&state->borrowrecords_start, state->borrowrecords,
                         kUserModifyMax, direction, "User", "User Modify");
  DrawUI(kUserModify, &user, state, msg);
}

void UserSearchInfoDisplay(User *show_user, char *msg) {
  // 更新这个用户的信息
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
  new_history->state.user_modify->frequency = GetBorrowRecordNumberAfter(
      borrow_record, time(NULL) - (time_t)0x28DE80);  // 2678400（31天）
  PushBackHistory(new_history);

  if (!msg) {
    msg = malloc(sizeof(char) * (30 + id_len + strlen(show_user->id)));
    sprintf(msg, "[Info] [%s] Show/Modify user [%s]", user.id, show_user->id);
  }
  Log(msg);
  DrawUI(kUserModify, &user, new_history->state.user_modify, msg);
}

void UserSearch_InfoCallback(User *show_user) {
  UserSearchInfoDisplay(show_user, NULL);
}

void UserSearchDisplay(char *keyword, char *msg) {
  if (user.whoami != ADMINISTRATOR) {
    char *msg = malloc(sizeof(char) * (49 + id_len));
    sprintf(msg, "[Error] [%s] Permission denied. Can't search users", user.id);
    ReturnHistory(history_list->dummy_tail->pre, msg);
    return;
  }

  // search
  List *results = NewList();
  char *true_keyword = malloc(sizeof(char) * (strlen(keyword) + 12));
  if (keyword[0] != '\0')
    sprintf(true_keyword, "%s&verified=1", keyword);
  else  // 不能以 & 开头
    sprintf(true_keyword, "verified=1");
  if (ErrorHandle(Filter(results, true_keyword, USER), 0)) {
    free(true_keyword);
    DeleteList(results, free);
    return;
  }
  free(true_keyword);

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

void UserSearch_SearchCallback(char *keyword) {
  char *new_keyword = malloc(sizeof(char) * (strlen(keyword) + 1));
  strcpy(new_keyword, keyword);
  UserSearchDisplay(new_keyword, NULL);
}

void UserSearch_TurnPage(bool direction) {
  UserSearch *state = TopHistory()->state.user_search;
  char *msg = MoveInList(&state->user_result_start, state->user_result,
                         kUserSearchMax, direction, "Result", "User Search");
  DrawUI(kUserSearch, &user, state, msg);
}

void LoginOrRegister_LoginCallback() {
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

    // 用户号不为空
    if (new_user->id[0] == '\0') {
      char *msg = malloc(sizeof(char) * 32);
      sprintf(msg, "[Error] id can't be blank");
      ReturnHistory(history_list->dummy_tail->pre, msg);
      return;
    }

    // 用户号不重复
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

    // 生成新用户
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

    // 在数据库里找出对应的用户
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

    // 验证密码
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

void UserManagement_ApproveCallback(ListNode *user_node, bool approve) {
  User *new_user = user_node->value;
  char *msg = malloc(sizeof(char) * (27 + id_len + strlen(new_user->id)));
  if (approve) {
    new_user->verified = TRUE;
    if (ErrorHandle(Update(new_user, new_user->uid, USER), 0)) {
      free(msg);
      return;
    }
    sprintf(msg, "[Info] [%s] Approve user [%s]", user.id, new_user->id);
  } else {
    if (ErrorHandle(Delete(new_user->uid, USER), 0)) {
      free(msg);
      return;
    }
    sprintf(msg, "[Info] [%s] Disprove user [%s]", user.id, new_user->id);
  }
  ReturnHistory(history_list->dummy_tail->pre, msg);
}

void UserManagement_DeleteCallback(ListNode *user_node) {
  User *new_user = user_node->value;
  // 管理员帐号不可删除
  if (new_user->whoami == ADMINISTRATOR) {
    char *msg = malloc(sizeof(char) * (38 + id_len));
    sprintf(msg, "[Error] [%s] Can't delete admin account", user.id);
    ReturnHistory(history_list->dummy_tail->pre, msg);
    return;
  }
  if (ErrorHandle(Delete(new_user->uid, USER), 0)) return;
  char *msg = malloc(sizeof(char) * (25 + id_len + strlen(new_user->id)));
  sprintf(msg, "[Info] [%s] Delete user [%s]", user.id, new_user->id);
  ReturnHistory(history_list->dummy_tail->pre, msg);
}

void UserManagement_TurnPage(bool direction, bool type) {
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

void UserManagement_InfoCallback(User *show_user) {
  UserSearchInfoDisplay(show_user, NULL);
}

void UserManagement_SortCallback(SortKeyword sort_keyword) {
  char *msg = malloc(sizeof(char) * (35 + id_len));
  switch (sort_keyword) {
    case kId:
      SortList(TopHistory()->state.user_management->users, CmpLessUserById);
      SortList(TopHistory()->state.user_management->to_be_verified,
               CmpLessUserById);
      sprintf(msg, "[Info] [%s] sort users by id", user.id);
      break;
    case kName:
      SortList(TopHistory()->state.user_management->users, CmpLessUserByName);
      SortList(TopHistory()->state.user_management->to_be_verified,
               CmpLessUserByName);
      sprintf(msg, "[Info] [%s] sort users by name", user.id);
      break;
    case kDepartment:
      SortList(TopHistory()->state.user_management->users,
               CmpLessUserByDepartment);
      SortList(TopHistory()->state.user_management->to_be_verified,
               CmpLessUserByDepartment);
      sprintf(msg, "[Info] [%s] sort users by department", user.id);
      break;
    default:
      Log("[Debug] Unknown sort_keyword in UserManagement_SortCallback");
      Error("Unknown sort_keyword");
      break;
  }
  TopHistory()->state.user_management->users_start =
      TopHistory()->state.user_management->users->dummy_head->nxt;
  TopHistory()->state.user_management->to_be_verified_start =
      TopHistory()->state.user_management->to_be_verified->dummy_head->nxt;
  Log(msg);
  DrawUI(kUserManagement, &user, TopHistory()->state.user_management, msg);
}

void BookDisplayAdminDisplay(char *msg) {
  if (user.whoami != ADMINISTRATOR) {
    char *msg = malloc(sizeof(char) * (60 + id_len));
    sprintf(msg,
            "[Error] [%s] Permission denied. Can't open Page BorrowDisplay",
            user.id);
    ReturnHistory(history_list->dummy_tail->pre, msg);
    return;
  }

  // 更新书的信息
  if (history_list->size ==
      0) {  // 有可能由于历史记录的个数上限，所需的信息已经丢失
    ReturnHistory(history_list->dummy_tail->pre, NULL);
    return;
  }
  if (ErrorHandle(GetById(TopHistory()->state.book_display->book,
                          TopHistory()->state.book_display->book->uid, BOOK),
                  0))
    return;

  // 获得这本书的借还记录
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
  new_history->state.borrow_display->frequency = GetBorrowRecordNumberAfter(
      borrow_record, time(NULL) - (time_t)0x28DE80);  // 2678400（31天）
  PushBackHistory(new_history);

  if (!msg) {
    msg = malloc(
        sizeof(char) *
        (46 + id_len + strlen(new_history->state.borrow_display->book_id)));
    sprintf(msg, "[Info] [%s] Open Page BorrowDisplay for book [%s]", user.id,
            new_history->state.borrow_display->book_id);
  }
  Log(msg);
  DrawUI(kBorrowDisplay, &user, new_history->state.borrow_display, msg);
}

void BookDisplay_AdminCallback() { BookDisplayAdminDisplay(NULL); }

void BookDisplay_CoverCallback() {
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

  // 将图片拷贝至 image 文件夹并加载图片
  const char *const book_id = TopHistory()->state.book_display->book->id;
  char *new_path = malloc(sizeof(char) * (image_dir_len + 6 + 10));
  sprintf(new_path, "%s\\%d.jpg", image_dir,
          TopHistory()->state.book_display->book->uid);
  char *msg = malloc(sizeof(char) * (46 + id_len + strlen(book_id)));
  if (!CopyFileA(image_path, new_path, FALSE)) {
    sprintf(msg, "[Error] [%s] Fail to change the book [%s]'s cover", user.id,
            book_id);
    free(new_path);
    DrawUI(TopHistory()->page, &user, TopHistory()->state.book_display, msg);
    return;
  }
  free(new_path);
  loadImage(image_path, &TopHistory()->state.book_display->book_cover);

  sprintf(msg, "[Info] [%s] Change the book [%s]'s cover", user.id, book_id);
  Log(msg);
  DrawUI(TopHistory()->page, &user, TopHistory()->state.book_display, msg);
}

void BookDisplay_ConfirmCallback() {
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

  // 书号不能为空
  Book *new_book = TopHistory()->state.book_display->book;
  if (new_book->id[0] == '\0') {
    char *msg = malloc(sizeof(char) * (36 + id_len));
    sprintf(msg, "[Error] [%s] Book's id can't be blank", user.id);
    ReturnHistory(history_list->dummy_tail->pre, msg);
    return;
  }

  // 书号不能重复
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

    char *msg = malloc(sizeof(char) * (25 + id_len + strlen(new_book->id)));
    sprintf(msg, "[Info] [%s] Init book [%s]", user.id, new_book->id);
    ReturnHistory(history_list->dummy_tail->pre, msg);
  } else {
    if (ErrorHandle(Update(new_book, new_book->uid, BOOK), 0)) return;

    char *msg = malloc(sizeof(char) * (25 + id_len + strlen(new_book->id)));
    sprintf(msg, "[Info] [%s] Modify book [%s]", user.id, new_book->id);
    ReturnHistory(history_list->dummy_tail->pre, msg);
  }
}

void BookDisplay_DeleteCallback() {
  if (user.whoami != ADMINISTRATOR) {
    char *msg = malloc(sizeof(char) * (53 + id_len));
    sprintf(msg, "[Error] [%s] Permission denied. Can't delete the book",
            user.id);
    ReturnHistory(history_list->dummy_tail->pre, msg);
    return;
  }
  Book *new_book = TopHistory()->state.book_display->book;

  // 删除数据库中的书
  if (TopHistory()->page == kBookModify) {
    if (ErrorHandle(Delete(new_book->uid, BOOK), 0)) return;
  }

  // 删除图书库文件夹中的封面
  char *cover_dir = malloc(sizeof(char) * (image_dir_len + 6 + 10));
  sprintf(cover_dir, "%s\\%d.jpg", image_dir, new_book->uid);
  DeleteFile(cover_dir);
  free(cover_dir);

  char *msg = malloc(sizeof(char) * (25 + id_len + strlen(new_book->id)));
  sprintf(msg, "[Info] [%s] Delete book [%s]", user.id, new_book->id);
  ReturnHistory(history_list->dummy_tail->pre->pre, msg);
}

void BookDisplay_BorrowCallback() {
  BookSearch_BorrowCallback(TopHistory()->state.book_display->book);
}

void BookDisplay_CopyPasteCallback() {
  Book *book = TopHistory()->state.book_display->book;
  const unsigned old_uid = book->uid;
  if (ErrorHandle(GetById(book, old_uid, BOOK), 0) ||
      ErrorHandle(GetNextPK(BOOK, &book->uid), 0)) {
    book->uid = old_uid;
    char *msg = malloc(sizeof(char) * (34 + id_len));
    sprintf(msg, "[Error] [%s] Fail to copy and paste", user.id);
    ReturnHistory(history_list->dummy_tail->pre, msg);
  }

  // 设置在架数为0并更新数据库
  book->number_on_the_shelf = 0;
  if (ErrorHandle(Create(book, BOOK), 0)) {
    book->uid = old_uid;
    char *msg = malloc(sizeof(char) * (34 + id_len));
    sprintf(msg, "[Error] [%s] Fail to copy and paste", user.id);
    ReturnHistory(history_list->dummy_tail->pre, msg);
  }
  const unsigned new_uid = book->uid;
  book->uid = old_uid;

  char *msg = NULL;

  // 复制图书封面
  char *old_image = malloc(sizeof(char) * (6 + 10 + image_dir_len));
  sprintf(old_image, "%s\\%d.jpg", image_dir, old_uid);
  char *new_image = malloc(sizeof(char) * (6 + 10 + image_dir_len));
  sprintf(new_image, "%s\\%d.jpg", image_dir, new_uid);
  if (!CopyFileA(old_image, new_image, FALSE)) {
    msg = malloc(sizeof(char) * (94 + id_len + strlen(book->id)));
    sprintf(msg,
            "[Error] [%s] Copy and paste book [%s], set number on the shelf to "
            "0, but fail to copy cover image",
            user.id, book->id);
  }
  free(old_image);
  free(new_image);

  // 新建一本书是防止历史记录遭到破坏
  Book new_book;
  new_book.uid = new_uid;

  if (!msg) {
    msg = malloc(sizeof(char) * (63 + id_len + strlen(book->id)));
    sprintf(
        msg,
        "[Info] [%s] Copy and paste book [%s], set number on the shelf to 0",
        user.id, book->id);
  }
  Navigation_BookDisplayOrInit(&new_book, 0, msg);
}

void BorrowDisplay_TurnPage(bool direction) {
  BorrowDisplay *state = TopHistory()->state.borrow_display;
  char *msg =
      MoveInList(&state->borrow_record_start, state->borrow_record,
                 kBorrowDisplayMax, direction, "Record", "Borrow Display");
  DrawUI(kBorrowDisplay, &user, state, msg);
}

void Library_BookCallback(ListNode *book) {
  Navigation_BookDisplayOrInit(book->value, 0, NULL);
}

void Library_SortCallback(SortKeyword sort_keyword) {
  // 由于图片模式不可排序，所以不需要对 book_covers 做处理
  char *msg = malloc(sizeof(char) * (33 + id_len));
  switch (sort_keyword) {
    case kId:
      SortList(TopHistory()->state.library->books, CmpLessBookById);
      sprintf(msg, "[Info] [%s] sort books by id", user.id);
      break;
    case kTitle:
      SortList(TopHistory()->state.library->books, CmpLessBookByTitle);
      sprintf(msg, "[Info] [%s] sort books by title", user.id);
      break;
    case kAuthor:
      SortList(TopHistory()->state.library->books, CmpLessBookByAuthor);
      sprintf(msg, "[Info] [%s] sort books by author", user.id);
      break;
    default:
      Log("[Debug] Unknown sort_keyword in Library_SortCallback");
      Error("Unknown sort_keyword");
      break;
  }
  TopHistory()->state.library->books_start =
      TopHistory()->state.library->books->dummy_head->nxt;
  Log(msg);
  DrawUI(kLibrary, &user, TopHistory()->state.library, msg);
}

void Library_SwitchCallback() {
  // 有可能由于历史记录的个数上限，所需的信息已经丢失
  if (history_list->size == 0 || TopHistory()->state.library->type == kList) {
    Navigation_Library(NULL);
  } else {
    // 获得图书库中的所有图书
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

void Statistics_SelectCallback(ListNode *catalog) {
  if (history_list->size ==
      0) {  // 有可能由于历史记录的个数上限，所需的信息已经丢失
    ReturnHistory(history_list->dummy_tail->pre, NULL);
    return;
  }
  List *borrow_records = NewList();
  Book *book = malloc(sizeof(Book));
  if (ErrorHandle(Filter(borrow_records, "", BORROWRECORD), 0)) {
    DeleteList(borrow_records, free);
    return;
  }
  if (strcmp(catalog->value, "ALL")) {  // 如果不是 ALL 这个分类，则进行筛选
    for (const ListNode *cur_node = borrow_records->dummy_head->nxt;
         cur_node != borrow_records->dummy_tail;) {
      if (ErrorHandle(
              GetById(book, ((BorrowRecord *)cur_node->value)->book_uid, BOOK),
              0)) {
        DeleteList(borrow_records, free);
        free(book);
        return;
      }
      if (strcmp(book->category, catalog->value))  // 分类不匹配的就删除
        cur_node = EraseList(borrow_records, cur_node, NULL);
      else
        cur_node = cur_node->nxt;
    }
  }
  SortList(borrow_records, CmpGreaterBorrowRecordByReturnTime);

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
  new_history->state.statistics->frequency = GetBorrowRecordNumberAfter(
      borrow_records, time(NULL) - (time_t)0x28DE80);  // 2678400（31天）
  PushBackHistory(new_history);

  char *msg = malloc(sizeof(char) * (46 + id_len + strlen(catalog->value)));
  sprintf(msg, "[Info] [%s] Open Page Statistics with category %s", user.id,
          (char *)catalog->value);
  Log(msg);
  DrawUI(kStatistics, &user, new_history->state.statistics, msg);
}

void Statistics_TurnPage(bool direction, bool type) {
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

void Navigation_LendAndBorrow(char *msg) {
  if (InitCheck(FALSE)) return;

  // 当前用户还没有归还的图书借阅记录
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

  // 对应的借阅记录的图书
  List *books = NewList();
  for (ListNode *cur_node = borrow_records_list->dummy_head->nxt;
       cur_node != borrow_records_list->dummy_tail; cur_node = cur_node->nxt) {
    Book *book = malloc(sizeof(Book));
    if (ErrorHandle(
            GetById(book, ((BorrowRecord *)cur_node->value)->book_uid, BOOK),
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

void Navigation_BookSearch(char *msg) {
  if (InitCheck(FALSE)) return;
  char *keyword = malloc(sizeof(char));
  *keyword = '\0';
  BookSearchDisplay(keyword, msg);
}

void Navigation_UserSearch(char *msg) {
  if (InitCheck(FALSE)) return;
  char *keyword = malloc(sizeof(char));
  *keyword = '\0';
  UserSearchDisplay(keyword, msg);
}

void Navigation_ManualOrAbout(bool type, char *msg) {
  History *const new_history = malloc(sizeof(History));

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
  DrawUI(new_history->page, &user, NULL, msg);
}

void Navigation_UserLogInOrRegister(bool type, char *msg) {
  if (InitCheck(TRUE)) return;

  // 登出当前用户
  memset(&user, 0x00, sizeof(User));
  id_len = 0;

  ClearHistory();  // 不能让下一个用户了解到上一个用户干了什么
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

void Navigation_UserLogOut(char *msg) {
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

void Navigation_UserModify(char *msg) {
  if (InitCheck(FALSE)) return;
  UserSearchInfoDisplay(&user, msg);
}

void Navigation_UserManagement(char *msg) {
  if (InitCheck(FALSE)) return;
  if (user.whoami != ADMINISTRATOR) {
    char *msg = malloc(sizeof(char) * (49 + id_len));
    sprintf(msg, "[Error] [%s] Permission denied. Can't manage users", user.id);
    ReturnHistory(history_list->dummy_tail->pre, msg);
    return;
  }

  // 待审核的用户列表
  List *to_be_verified = NewList();
  if (ErrorHandle(Filter(to_be_verified, "verified=0", USER), 0)) {
    DeleteList(to_be_verified, free);
    return;
  }

  // 已审核有效的用户列表
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
  new_history->state.user_management->info_callback =
      UserManagement_InfoCallback;
  new_history->state.user_management->sort_callback =
      UserManagement_SortCallback;
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

void Navigation_Library(char *msg) {
  if (InitCheck(FALSE)) return;
  List *books = NewList();
  if (ErrorHandle(Filter(books, "", BOOK), 0)) {
    DeleteList(books, free);
    return;
  }

  List *book_covers = NewList();
  // 由于图片模式被砍，先注释掉
  // char *image_path = malloc(sizeof(char) * (image_dir_len + 12));
  // sprintf(image_path, "%s\\", image_dir);
  // for (ListNode *cur_node = books->dummy_head->nxt;
  //      cur_node != books->dummy_tail; cur_node = cur_node->nxt) {
  //   LibImage *image = malloc(sizeof(LibImage));
  //   sprintf(image_path + image_dir_len + 1, "%d.jpg",
  //           ((Book *)cur_node->value)->uid);
  //   if (!_access(image_path, 4))
  //     loadImage(image_path, image);
  //   else
  //     copyImage(image, &unknown_cover);
  //   InsertList(book_covers, book_covers->dummy_tail, image);
  // }

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
    // sprintf(msg, "[Info] [%s] Open Page Library (image mode)", user.id);
    sprintf(msg, "[Info] [%s] Open Page Library",
            user.id);  // 由于图片模式被砍，暂时这么处理
  }
  Log(msg);
  DrawUI(kLibrary, &user, new_history->state.library, msg);
}

void Navigation_OpenOrInitLibrary(bool type, char *msg) {
  // copy *.swp.db to *.db and remove *.swp.db
  static bool opened = FALSE;  // 表示之前是否打开过某个数据库
  if (opened) Navigation_SaveLibrary(0, NULL);

  try {
    SelectFolder("请选择保存图书库的文件夹", lib_dir);
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
  lib_dir_len = strlen(lib_dir);

  // 关闭老的数据库
  if (ErrorHandle(CloseDBConnection(USER), 1, DB_NOT_OPEN)) return;
  DeleteFile(user_db_dir);

  db_open = FALSE;

  if (ErrorHandle(CloseDBConnection(BOOK), 1, DB_NOT_OPEN)) return;
  DeleteFile(book_db_dir);

  if (ErrorHandle(CloseDBConnection(BORROWRECORD), 1, DB_NOT_OPEN)) return;
  DeleteFile(borrowrecord_db_dir);

  sprintf(image_dir, "%s\\image", lib_dir);
  image_dir_len = strlen(image_dir);
  if (type) {  // 新建图片文件夹
    if (!CreateDirectory(image_dir, NULL)) {
      char *msg = malloc(sizeof(char) * (63 + id_len));
      sprintf(
          msg,
          "[Error] [%s] Fail to init the library. Can't create image folder",
          user.id);
      ReturnHistory(history_list->dummy_tail->pre, msg);
      return;
    }
  }

  int flag = 0;  // 0 => 无事发生 1=> 有swap文件 2=> 无文件

  // 打开数据库
  sprintf(user_db_dir, "%s%s", lib_dir, "\\user.swp.db");
  if (!_access(user_db_dir, 6)) {  // swap file exists
    flag |= 1;
  } else {
    char *user_database_path = malloc(sizeof(char) * (lib_dir_len + 9));
    sprintf(user_database_path, "%s\\user.db", lib_dir);
    if (!_access(user_database_path, 6))  // user database exists
      CopyFileA(user_database_path, user_db_dir, FALSE);
    else  // book database doesn't exist
      flag |= 2;
    free(user_database_path);
  }
  if (ErrorHandle(OpenDBConnection(user_db_dir, USER), 2, DB_NOT_OPEN,
                  DB_ENTRY_EMPTY))
    return;

  sprintf(book_db_dir, "%s%s", lib_dir, "\\book.swp.db");
  if (!_access(book_db_dir, 6)) {  // swap file exists
    flag |= 1;
  } else {
    char *book_database_path = malloc(sizeof(char) * (lib_dir_len + 9));
    sprintf(book_database_path, "%s\\book.db", lib_dir);
    if (!_access(book_database_path, 6))  // book database exists
      CopyFileA(book_database_path, book_db_dir, FALSE);
    else  // book database doesn't exist
      flag |= 2;
    free(book_database_path);
  }
  if (ErrorHandle(OpenDBConnection(book_db_dir, BOOK), 2, DB_NOT_OPEN,
                  DB_ENTRY_EMPTY)) {
    CloseDBConnection(USER);
    return;
  }

  sprintf(borrowrecord_db_dir, "%s%s", lib_dir, "\\borrowrecord.swp.db");
  if (!_access(borrowrecord_db_dir, 6)) {  // swap file exists
    flag |= 1;
  } else {
    char *borrowrecord_database_path =
        malloc(sizeof(char) * (lib_dir_len + 17));
    sprintf(borrowrecord_database_path, "%s\\borrowrecord.db", lib_dir);
    if (!_access(borrowrecord_database_path,
                 6))  // borrowrecord database exists
      CopyFileA(borrowrecord_database_path, borrowrecord_db_dir, FALSE);
    else  // borrowrecord database doesn't exist
      flag |= 2;
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
    msg = malloc(sizeof(char) * (114 + lib_dir_len + id_len));
    if (type) {
      if (flag != 2)
        sprintf(msg,
                "[Error] [%s] Log out, Clear history and init library from %s, "
                "where "
                "already exists an eLibrary",
                user.id, lib_dir);
      else
        sprintf(msg,
                "[Info] [%s] Log out, Clear history and init library from %s",
                user.id, lib_dir);
    } else {
      if ((flag & 2) == 2) {
        sprintf(msg,
                "[Error] [%s] Log out, Clear history and fail to open library "
                "from %s, "
                "init one at there, undefined behavior may occur",
                user.id, lib_dir);
        CreateDirectory(image_dir, NULL);
      } else if ((flag & 1) == 1) {
        sprintf(msg,
                "[Warning] [%s] Log out, Clear history and open library from "
                "%s using "
                "swap file",
                user.id, lib_dir);
      } else if (!flag) {
        sprintf(msg,
                "[Info] [%s] Log out, Clear history and open library from %s",
                user.id, lib_dir);
      }
    }
  }

  db_open = TRUE;
  opened = TRUE;
  memset(&user, 0x00, sizeof(User));
  id_len = 0;

  Log(msg);
  DrawUI(kWelcome, &user, NULL, msg);
}

void Navigation_SaveLibrary(bool type, char *msg) {
  if (InitCheck(TRUE)) return;

  // 由于现在数据库不是及时写入的，所以先这么办
  if (ErrorHandle(CloseDBConnection(USER), 0)) return;
  if (ErrorHandle(CloseDBConnection(BOOK), 0)) return;
  if (ErrorHandle(CloseDBConnection(BORROWRECORD), 0)) return;
  db_open = FALSE;

  // 复制 swap 文件
  char *user_database_path = malloc(sizeof(char) * (lib_dir_len + 9));
  sprintf(user_database_path, "%s\\user.db", lib_dir);
  CopyFileA(user_db_dir, user_database_path, FALSE);
  free(user_database_path);

  char *book_database_path = malloc(sizeof(char) * (lib_dir_len + 9));
  sprintf(book_database_path, "%s\\book.db", lib_dir);
  CopyFileA(book_db_dir, book_database_path, FALSE);
  free(book_database_path);

  char *borrowrecord_database_path = malloc(sizeof(char) * (lib_dir_len + 17));
  sprintf(borrowrecord_database_path, "%s\\borrowrecord.db", lib_dir);
  CopyFileA(borrowrecord_db_dir, borrowrecord_database_path, FALSE);
  free(borrowrecord_database_path);

  // 由于现在数据库不是及时写入的，所以先这么办
  if (ErrorHandle(OpenDBConnection(user_db_dir, USER), 0)) return;
  if (ErrorHandle(OpenDBConnection(book_db_dir, BOOK), 0)) return;
  if (ErrorHandle(OpenDBConnection(borrowrecord_db_dir, BORROWRECORD), 0))
    return;
  db_open = TRUE;

  if (type) {
    if (!msg) {
      msg = malloc(sizeof(char) * (23 + id_len));
      sprintf(msg, "[Info] [%s] Save library", user.id);
    }
    ReturnHistory(history_list->dummy_tail->pre, msg);
  }
}

void Navigation_BookDisplayOrInit(Book *book, bool type, char *msg) {
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
  new_history->state.book_display->copy_paste_callback =
      BookDisplay_CopyPasteCallback;
  new_history->state.book_display->book = new_book;
  // 加载要显示的封面图片
  if (!type) {  // 图书显示
    char *image_path = malloc(sizeof(char) * (6 + image_dir_len + 10));
    sprintf(image_path, "%s\\%d.jpg", image_dir, book->uid);
    if (!_access(image_path, 4))  // 找的到图书封面
      loadImage(image_path, &new_history->state.book_display->book_cover);
    else  // 找不到图书封面
      copyImage(&new_history->state.book_display->book_cover, &unknown_cover);
  } else {  // 图书新建
    copyImage(&new_history->state.book_display->book_cover, &edit_cover);
  }
  PushBackHistory(new_history);

  if (!msg) {
    if (type) {
      msg = malloc(sizeof(char) * (30 + id_len));
      sprintf(msg, "[Info] [%s] Open book init page", user.id);
    } else {
      msg = malloc(sizeof(char) * (36 + id_len + strlen(new_book->id)));
      if (user.whoami == ADMINISTRATOR)
        sprintf(msg, "[Info] [%s] Open book modify page [%s]", user.id,
                new_book->id);
      else
        sprintf(msg, "[Info] [%s] Open book display page [%s]", user.id,
                new_book->id);
    }
  }
  Log(msg);
  DrawUI(new_history->page, &user, new_history->state.book_display, msg);
}

void Navigation_BookInit(char *msg) {
  if (InitCheck(FALSE)) return;
  Book *book = malloc(sizeof(Book));
  memset(book, 0, sizeof(Book));
  if (ErrorHandle(GetNextPK(BOOK, &book->uid), 0)) {
    free(book);
    return;
  }
  Navigation_BookDisplayOrInit(book, 1, msg);
}

void Navigation_Statistics(char *msg) {
  if (InitCheck(FALSE)) return;
  if (user.whoami != ADMINISTRATOR) {
    char *msg = malloc(sizeof(char) * (57 + id_len));
    sprintf(msg, "[Error] [%s] Permission denied. Can't open Page Statistics",
            user.id);
    ReturnHistory(history_list->dummy_tail->pre, msg);
    return;
  }

  List *book = NewList(), *category = NewList();
  // 获得所有书
  if (ErrorHandle(Filter(book, "", BOOK), 0)) {
    DeleteList(book, free);
    DeleteList(category, free);
    return;
  }
  // 提取出所有分类
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

  // 加入"全部"这一分类
  char *str = malloc(sizeof(char) * 4);
  sprintf(str, "ALL");
  InsertList(category, category->dummy_head->nxt, str);

  // 获得所有借阅记录
  List *borrow_record = NewList();
  if (ErrorHandle(Filter(borrow_record, "", BORROWRECORD), 0)) {
    DeleteList(category, free);
    DeleteList(borrow_record, free);
    return;
  }
  SortList(borrow_record, CmpGreaterBorrowRecordByReturnTime);

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
  new_history->state.statistics->frequency = GetBorrowRecordNumberAfter(
      borrow_record, time(NULL) - (time_t)0x28DE80);  // 2678400（31天）
  PushBackHistory(new_history);

  if (!msg) {
    msg = malloc(sizeof(char) * (31 + id_len));
    sprintf(msg, "[Info] [%s] Open Page Statistics", user.id);
  }
  Log(msg);
  DrawUI(kStatistics, &user, new_history->state.statistics, msg);
}

void Navigation_Return(char *msg) {
  if (history_list->size < 2) {  // 没有可以返回的界面
    if (!msg) {
      msg = malloc(sizeof(char) * (44 + id_len));
      sprintf(msg, "[Error] [%s] There's no history to go back to", user.id);
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

void Navigation_Exit() {
  Navigation_SaveLibrary(0, NULL);

  // 关闭数据库，删除 swap 文件
  if (ErrorHandle(CloseDBConnection(USER), 1, DB_NOT_OPEN)) exit(1);
  DeleteFile(user_db_dir);

  if (ErrorHandle(CloseDBConnection(BOOK), 1, DB_NOT_OPEN)) exit(1);
  DeleteFile(book_db_dir);

  if (ErrorHandle(CloseDBConnection(BORROWRECORD), 1, DB_NOT_OPEN)) exit(1);
  DeleteFile(borrowrecord_db_dir);

  UninitHistory();

  Log("[Info] Shutdown");
  UninitUtility();

  ExitGraphics();
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
