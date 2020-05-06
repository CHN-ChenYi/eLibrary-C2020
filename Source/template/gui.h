#include "graphics.h"
#include "basictype.h"
#include "list.h"

/*
 * 界面绘制函数
 * cur_page: 当前界面（当前的用户
 * cur_user: 当前登陆的用户（若为NULL，则未登录）
 *     info: 一个结构体指针，其类型由cur_page决定
 * terminal: 终端输出
 */
void DrawUI (Page cur_page, User* cur_user, void* info, char* terminal);

/*
 * 顶部菜单点击
 * nav_page: 导航到哪个界面
 * cur_user: 当前登陆的界面
 *     info: 当前界面的信息  
 * terminal: 终端输出
 */
void NavigationCallback (Page nav_page, User* cur_user, void* info, char* terminal);

typedef enum {
  kLendAndBorrow,       // 借还书界面
  kSearch,              // 搜索结果界面
  kManual,              // 帮助/关于界面
  kUserLogIn,           // 用户登陆界面
  kUserModify,          // 用户信息修改
  kUserManagement,      // 用户管理界面（管理员）
  kLibrary,             // 图书库界面
  kBookDisplay,         // 图书显示/新增/修改界面
  kBorrowDisplay,       // 借还书界面（管理员）
  kStatistics,          // 统计界面
  kInitLibrary,         // 新建图书库
  kOpenLibrary,         // 打开图书库
  kReturn               // 回到上一个界面
} Page;

/* 借书还书界面 */
struct LendAndBorrow {
  enum {kLend, kBorrow} Type;                 // 借书还是还书
  Book *book;                                 // 书籍信息
  List *book_list;                            // 待借/还书列表
  void (*search_callback) (char* keyword);    // 搜索按钮
  void (*button_callback) ();                 // 确定/放弃按钮
  void (*return_callback) ();                 // 返回按钮
};

/* 搜索界面 */
struct Search {
  char *keyword;                              // 搜索关键词
  List *search_result;                        // 结果链表
  void (*search_callback) (char* keyword);    // 搜索按钮
  void (*return_callback) ();                 // 返回按钮
};

/* 用户手册/关于界面 */
struct Manual {
  enum {kManual, kAbout} Type;    // 用户手册/关于
  char *title;
  char *content;
  void (*return_callback) ();     // 返回按钮
};

/* 用户登陆/注册界面 */
struct UserLogIn {
  enum {kLogin, kRegister} Type;  // 登录还是注册
  User *user;                      // 当前正在登陆/注册的这个用户
  char password[50];              // 注册/登陆的密码
  char repeat_password[50];       // 重复新密码
  void (*login_callback) ();      // 登陆/注册按钮
  void (*return_callback) ();     // 返回按钮
};

/* 用户信息修改 */
struct UserModify {
  User *user;                    // 用户信息
  char old_password[50];        // 旧密码
  char new_password[50];        // 新密码
  char repeat_password[50];     // 重复新密码
  void (*confirm_callback) ();  // 确认按钮
  void (*admin_callback) ();    // 管理员修改他人账户按钮
  void (*return_callback) ();   // 返回按钮
};


/* 审核、修改、删除用户 */
struct UserManagement {
  List *to_be_verified;               // 待审核用户列表
  List *users;                        // 已添加用户列表
  void (*callback) (ListNode* user);  // 审核通过或者拒绝
  void (*return_callback) ();         // 返回按钮
};

/* 图书库界面 */
struct Library {
  enum {kPicture, kList} Type;               // 图片模式还是列表模式
  List *books;                              // 图书库的图书
  void (*sort_callback) ();                 // 排序按钮
  void (*book_callback) (ListNode* book);   // 图书详细信息按钮
  void (*return_callback) ();               // 返回按钮
};

/* 图书显示、新建、修改 */
struct BookDisplay {
  enum {kDisplay, kInit, kModify} Type;   // 当前状态：显示/新建/删除
  Book *book;                              // 当前书籍
  void (*admin_callback) ();              // 查看图书借阅次数按钮（管理员）
  void (*return_callback) ();             // 返回按钮
};

/* 图书借还界面显示 */
struct BorrowDisplay {
  char *book_name;                    // 当前书籍
  List *borrow_record;                // 当前书籍的借还记录
  void (*confirm_callback) ();        // 确认按钮
  void (*return_callback) ();         // 返回按钮
};

/* 统计界面 */
struct Statistics {
  List *borrow_record;                // 借还次数统计
  void (*return_callback) ();         // 返回按钮
};