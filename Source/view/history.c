#include "history.h"

#include "model.h"
#include "utility.h"
#include "view.h"

extern User user;
extern size_t id_len;
List *history_list;

void InitHistory() { history_list = NewList(); }

void UninitHistory() {
  ClearHistory();
  DeleteList(history_list, free);
}

History *const TopHistory() {
  return (History *)history_list->dummy_tail->pre->value;
}

void PushBackHistory(History *const new_history) {
  InsertList(history_list, history_list->dummy_tail, new_history);
  // if the number of history exceed max_size, delete some of the histories
  while (history_list->size > HISTORY_MAX)
    EraseList(history_list, history_list->dummy_head->nxt, FreeHistory);
}

void PopBackHistory() {
  EraseList(history_list, history_list->dummy_tail->pre, FreeHistory);
}

void ClearHistory() { ClearList(history_list, FreeHistory); }

void FreeHistory(void *const history_) {
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

void ReturnHistory(ListNode *go_back_to, char *msg) {
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
      char *keyword = malloc(sizeof(char) *
                             (strlen(history->state.book_search->keyword) + 1));
      strcpy(keyword, history->state.book_search->keyword);
      PopBackHistory();
      BookSearchDisplay(keyword, msg);
    } break;
    case kUserSearch: {  // 用户搜索（管理员）
      char *keyword = malloc(sizeof(char) *
                             (strlen(history->state.user_search->keyword) + 1));
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
      if (ErrorHandle(
              GetById(new_user, history->state.user_modify->user->uid, USER),
              0))
        return;
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
