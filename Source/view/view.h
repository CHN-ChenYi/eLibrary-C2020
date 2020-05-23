#ifndef VIEW_H_
#define VIEW_H_

#include "genlib.h"
#include "gui.h"

void InitView();
void NavigationCallback(Page nav_page);

void Navigation_LendAndBorrow(char *msg);
void Navigation_BookSearch(char *msg);
void Navigation_UserSearch(char *msg);
void Navigation_ManualOrAbout(bool type, char *msg);
void Navigation_UserLogInOrRegister(bool type, char *msg);
void Navigation_UserLogOut(char *msg);
void Navigation_UserModify(char *msg);
void Navigation_UserManagement(char *msg);
void Navigation_Library(char *msg);
void Navigation_OpenOrInitLibrary(bool type, char *msg);
void Navigation_SaveLibrary(bool type, char *msg);
void Navigation_BookDisplayOrInit(Book *book, bool type, char *msg);
void Navigation_BookInit(char *msg);
void Navigation_Statistics(char *msg);
void Navigation_Return(char *msg);
void Navigation_Exit();

void BookDisplayAdminDisplay(char *msg);
void BookSearchDisplay(char *keyword, char *msg);
void UserSearchDisplay(char *keyword, char *msg);
void UserSearchInfoDisplay(User *show_user, char *msg);

#endif  // !VIEW_H_
