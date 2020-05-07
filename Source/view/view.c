#include "view.h"

typedef union State {
  LendAndBorrow lend_and_borrow;
  BookSearch book_search;
  UserSearch user_search;
  ManualAndAbout manual_and_about;
  LoginOrRegister login_or_register;
  UserModify user_modify;
  UserManagement user_management;
  Library library;
  BookDisplay book_display;
  BorrowDisplay borrow_display;
  Statistics statistics;
} State;

typedef struct History {
  Page page;
  State state;
} History;

void DrawUI(Page cur_page, User *cur_user, void *info, char *terminal) {

}


void NavigationCallback(Page nav_page, User *cur_user, char *terminal) {

}