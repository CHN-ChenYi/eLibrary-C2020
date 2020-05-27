#ifndef VIEW_H_
#define VIEW_H_

#include "genlib.h"
#include "gui.h"

void InitView();
void NavigationCallback(Page nav_page);  // 响应用户导航栏的操作

// 处理对应界面需要显示的东西并调用绘制函数，
// 如果 msg 不为 NULL 则 terminal 直接显示 msg
void Navigation_LendAndBorrow(char *msg);
void Navigation_BookSearch(char *msg);
void Navigation_UserSearch(char *msg);
void Navigation_ManualOrAbout(bool type, char *msg);  // type = 0 => Manual
void Navigation_UserLogInOrRegister(bool type, char *msg);  // type = 0 => LogIn
void Navigation_UserLogOut(char *msg);
void Navigation_UserModify(char *msg);
void Navigation_UserManagement(char *msg);
void Navigation_Library(char *msg);
void Navigation_OpenOrInitLibrary(bool type, char *msg);  // type == 0 => Open
// type = 0 => 不回退到上一个界面
void Navigation_SaveLibrary(bool type, char *msg);
// type = 0 => Display
void Navigation_BookDisplayOrInit(Book *book, bool type, char *msg);
void Navigation_BookInit(char *msg);
void Navigation_Statistics(char *msg);
void Navigation_Return(char *msg);
void Navigation_Exit();

void BookDisplayAdminDisplay(char *msg);  // 绘制 BorrowDisplay 界面
void BookSearchDisplay(char *keyword, char *msg);
void UserSearchDisplay(char *keyword, char *msg);
// 绘制指定用户的详细信息
void UserSearchInfoDisplay(User *show_user, char *msg);

#endif  // !VIEW_H_
