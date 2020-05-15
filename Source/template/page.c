#include "ui.h"
#include "gui.h"
#include "graphics.h"
#include "extgraph.h"
#include "page.h"
#include "list.h"

typedef enum {
  kUnclicked,
  kFile,
  kBook,
  kUser,
  kHelp,
  kSearch
} HeadBarStatus;

HeadBarStatus hb_status;

// Current Page
static Page cur_page;
// Current state
static void* cur_info;
static User* cur_user;
static char cur_terminal[500];

/* HeadBar */

// id for head bar items
#define FILE_ID -1
#define BOOK_ID -2
#define LEND_ID -3
#define USER_ID -4
#define SEARCH_ID -5
#define HELP_ID -6
#define ST_ID -7
#define RTN_ID -8
#define NULL_ID 0     // will not triger any callback

#define MENU_COLOR "BDBDBD"
#define CONFIRM_COLOR "757575"
#define SEARCH_COLOR "EF5350"
#define PANEL_COLOR "E0E0E0"

/* Headbar menu */
static Link *HelpButton, *FileButton, *BookButton,
            *LendAndBorrowButton, *UserButton,
            *SearchButton, *Quit, *Statistic;
static Button *return_button;
static Label *bottom_output, *bottom_info;

/* Background for head bar and foot bar */
static Frame *hb_frame, *fb_frame;

/* Submenu */

// submenu for File
static Button *init_lib, *open_lib, *save_lib, *quit;
// submenu for Book
static Button *new_book, *display_book;
// submenu for User
static Button *login, *new_user, *varify, *logout;
// submenu for Help
static Button *about, *manual;
// submenu for Search
static Button *search_user, *search_book;

/*
 * Callback id:
 * 201 用户注册确认
 * 301 用户修改确认
 * 401 登陆
 * 501 按照ID排序
 * 502 按照title排序
 * 503 按照author排序
 */

void ExitSurface() {
  hb_status = kUnclicked;
  InitSurface();
}

void AddSubmenuFile() {
  init_lib = CreateButton(
    (Rect) {0, 150, 70, 110},
    "新建", MENU_COLOR, 1, kWhite, 1
  );
  open_lib = CreateButton(
    (Rect) {0, 150, 110, 150},
    "打开", MENU_COLOR, 1, kWhite, 2
  );
  save_lib = CreateButton(
    (Rect) {0, 150, 150, 190},
    "保存", MENU_COLOR, 1, kWhite, 3
  );
  quit = CreateButton(
    (Rect) {0, 150, 190, 230},
    "退出", MENU_COLOR, 1, kWhite, 4
  );
  InsertSurface(init_lib, kButton);
  InsertSurface(open_lib, kButton);
  InsertSurface(save_lib, kButton);
  InsertSurface(quit, kButton);
}

void AddSubmenuBook() {
  new_book = CreateButton(
    (Rect) {70, 220, 70, 110},
    "新建", MENU_COLOR, 1, kWhite, 1
  );
  display_book = CreateButton(
    (Rect) {70, 220, 110, 150},
    "显示", MENU_COLOR, 1, kWhite, 2
  );
  InsertSurface(new_book, kButton);
  InsertSurface(display_book, kButton);
}

void AddSubmenuUser() {
  login = CreateButton(
    (Rect) {200, 350, 70, 110},
    "登录", MENU_COLOR, 1, kWhite, 1
  );
  new_user = CreateButton(
    (Rect) {200, 350, 110, 150},
    "新建", MENU_COLOR, 1, kWhite, 2
  );
  varify = CreateButton(
    (Rect) {200, 350, 150, 190},
    "审核", MENU_COLOR, 1, kWhite, 3
  );
  logout = CreateButton(
    (Rect) {200, 350, 190, 230},
    "登出", MENU_COLOR, 1, kWhite, 4
  );
  InsertSurface(login, kButton);
  InsertSurface(new_user, kButton);
  InsertSurface(varify, kButton);
  InsertSurface(logout, kButton);
}

void AddSubmenuHelp() {
  about = CreateButton(
    (Rect) {GetWindowWidthPx() - 150, GetWindowWidthPx(), 70, 110},
    "关于", MENU_COLOR, 1, kWhite, 1
  );
  manual = CreateButton(
    (Rect) {GetWindowWidthPx() - 150, GetWindowWidthPx(), 110, 150},
    "用户手册", MENU_COLOR, 1, kWhite, 2
  );
  InsertSurface(about, kButton);
  InsertSurface(manual, kButton);
}

void AddSubmenu(int status) {
  switch (status) {
    case FILE_ID:
      AddSubmenuFile();
      hb_status = kFile;
      break;
    case BOOK_ID:
      AddSubmenuBook();
      hb_status = kBook;
      break;
    case USER_ID:
      AddSubmenuUser();
      hb_status = kUser;
      break;
    case HELP_ID:
      AddSubmenuHelp();
      hb_status = kHelp;
      break;
  }
}

// TODO(Hans): UserName to be tested
void AddHeadBar() {
  // Head bar
  FileButton = CreateLink(
    (Rect) {15, 75, 5, 45},
    "文件", kWhite, FILE_ID
  );
  BookButton = CreateLink(
    (Rect) {85, 145, 5, 45},
    "图书", kWhite, BOOK_ID
  );
  LendAndBorrowButton = CreateLink(
    (Rect) {155, 215, 5, 45},
    "借还", kWhite, LEND_ID
  );
  UserButton = CreateLink(
    (Rect) {225, 285, 5, 45},
    "用户", kWhite, USER_ID
  );
  Label *user_name = NULL;
  if (cur_user == NULL) {
    user_name = CreateLabel(
      (Rect){295, 355, 5, 45}, "未登录", kBlack, NULL_ID
    );
  } else {
    user_name = CreateLabel(
      (Rect){295, 355, 5, 45}, cur_user->username, kBlack, NULL_ID
    );
  }
  SearchButton = CreateLink(
    (Rect) {295, 355, 5, 45},
    "搜索", kWhite, SEARCH_ID
  );
  Statistic = CreateLink(
    (Rect) {365, 425, 5, 45},
    "统计", kWhite, ST_ID
  );
  return_button = CreateButton(
    (Rect) {435, 495, 15, 55},
    "返回", "757575", 1, kWhite, RTN_ID
  );
  HelpButton = CreateLink(
    (Rect) {GetWindowWidthPx() - 60, 0, 5, 45},
    "帮助", kWhite, HELP_ID
  );
  hb_frame = CreateFrame(
    (Rect) {0, GetWindowWidthPx(), 0, 70},
    "2979FF", 0.5
  );
  InsertFrame(hb_frame);
  InsertComp(HelpButton, kLink);
  InsertComp(return_button, kButton);
  InsertComp(Statistic, kLink);
  InsertComp(SearchButton, kLink);
  InsertComp(UserButton, kLink);
  InsertComp(LendAndBorrowButton, kLink);
  InsertComp(BookButton, kLink);
  InsertComp(FileButton, kLink);
}

void AddFooBar() {
  // foo bar
  bottom_output = CreateLabel (
    (Rect) {5, GetWindowWidthPx(), GetWindowHeightPx() - 50, GetWindowHeightPx() - 15},
    "状态栏输出：", kWhite, NULL_ID
  );
  fb_frame = CreateFrame(
    (Rect) {0, GetWindowWidthPx(), GetWindowHeightPx() - 50, GetWindowHeightPx()},
    "212121", 0.5
  );
  bottom_info = CreateLabel (
    (Rect) {TextStringWidth("状态栏输出：") + 5, GetWindowWidthPx(), GetWindowHeightPx() - 50, GetWindowHeightPx() - 15},
    cur_terminal, kWhite, NULL_ID
  );
  InsertFrame(fb_frame);
  InsertComp(bottom_output, kLabel);
  InsertComp(bottom_info, kLabel);
}

// 
void AddLendAndBorrow() {
  /*
   * ID configuration
   * 101: search book
   * 1 - n: return book
   */
  Label *return_title = CreateLabel(
    (Rect){27, 100, 75, 100}, "还书：", kBlack, NULL_ID
  );
  Label *borrow_title = CreateLabel(
    (Rect){27, 100, GetWindowHeightPx() - 200, GetWindowHeightPx() - 100},
    "借书：", kBlack, NULL_ID
  );
  InputBox *input_box = CreateInputBox(
    (Rect){80, 250, GetWindowHeightPx() - 200, GetWindowHeightPx() - 100}, "", NULL_ID
  );
  Button *search_button = CreateButton(
    (Rect){260, 380, GetWindowHeightPx() - 120, GetWindowHeightPx() - 90}, 
    "搜索", SEARCH_COLOR, 1, kWhite, 101
  );
  InsertComp(search_button, kButton);
  InsertComp(input_box, kInputBox);
  InsertComp(return_title, kLabel);
  InsertComp(borrow_title, kLabel);
}

// TODO(Hans), Book search to be tested
void AddBookSearch() {
  /*
   * ID configuration:
   * 101: search
   */
  State cur_state;
  cur_state.book_search = cur_info;
  Label *search_title = CreateLabel(
    (Rect){30, 0, 0, 120}, "搜索结果：", kBlack, NULL_ID
  );
  Label *search_info = CreateLabel(
    (Rect){30 + TextStringWidth("搜索结果："), 0, 0, 120},
    cur_state.book_search->keyword, kBlack, NULL_ID
  );
  InputBox *input_box = CreateInputBox(
    (Rect){50, 350, 0, 150}, "", NULL_ID
  );
  Button *button = CreateButton(
    (Rect){400, 500, 120, 160}, "搜索", SEARCH_COLOR, 1, kBlack, 101
  );
  InsertComp(button, kButton);
  InsertComp(search_title, kLabel);
  InsertComp(search_info, kLabel);
  InsertComp(input_box, kInputBox);
}

// TODO(Hans), User search to be tested
void AddUserSearch() {
  /*
   * ID configuration:
   * 101: search
   */
  State cur_state;
  cur_state.user_search = cur_info;
  Label *search_title = CreateLabel(
    (Rect){30, 0, 0, 120}, "搜索结果：", kBlack, NULL_ID
  );
  Label *search_info = CreateLabel(
    (Rect){30 + TextStringWidth("搜索结果："), 0, 0, 120},
    cur_state.user_search->keyword, kBlack, NULL_ID
  );
  InputBox *input_box = CreateInputBox(
    (Rect){50, 350, 0, 150}, "", NULL_ID
  );
  Button *button = CreateButton(
    (Rect){400, 500, 120, 160}, "搜索", SEARCH_COLOR, 1, kBlack, 101
  );
  InsertComp(button, kButton);
  InsertComp(search_title, kLabel);
  InsertComp(search_info, kLabel);
  InsertComp(input_box, kInputBox);
}

void AddUserRegister() {
  /*
   * ID configuration
   * 201: confirm
   */
  int pos_x = GetWindowWidthPx() / 2 - 200;
  int pos_y = GetWindowHeightPx() / 2 - 250;
  Frame* center_frame = CreateFrame(
    (Rect){pos_x, pos_x + 400, pos_y, pos_y + 500},
    PANEL_COLOR, 1
  );
  InsertFrame(center_frame);
  Label *register_title = CreateLabel(
    (Rect){pos_x + 5, 0, 0, pos_y + 25}, "用户注册：", kBlack, NULL_ID
  );
  Label *username_label = CreateLabel(
    (Rect){pos_x + 15, 0, 0, pos_y + 70}, "用户名：", kBlack, NULL_ID
  );
  InputBox* username_input = CreateInputBox(
    (Rect){pos_x + 150, pos_x + 350, 0, pos_y + 70}, "", NULL_ID
  );
  Label *first_pw_label = CreateLabel(
    (Rect){pos_x + 15, 0, 0, pos_y + 130}, "密码：", kBlack, NULL_ID
  );
  InputBox* first_pw_input = CreateInputBox(
    (Rect){pos_x + 150, pos_x + 350, 0, pos_y + 130}, "", NULL_ID
  );
  Label *second_pw_label = CreateLabel(
    (Rect){pos_x + 15, 0, 0, pos_y + 190}, "重复密码：", kBlack, NULL_ID
  );
  InputBox* second_pw_input = CreateInputBox(
    (Rect){pos_x + 150, pos_x + 350, 0, pos_y + 190}, "", NULL_ID
  );
  Label *dpt_label = CreateLabel(
    (Rect){pos_x + 15, 0, 0, pos_y + 250}, "部门：", kBlack, NULL_ID
  );
  InputBox* dpt_input = CreateInputBox(
    (Rect){pos_x + 150, pos_x + 350, 0, pos_y + 250}, "", NULL_ID
  );
  Label *sex_label = CreateLabel(
    (Rect){pos_x + 15, 0, 0, pos_y + 310}, "性别（M/F）", kBlack, NULL_ID
  );
  InputBox* sex_input = CreateInputBox(
    (Rect){pos_x + 150, pos_x + 350, 0, pos_y + 310}, "", NULL_ID
  );
  Label *admin_label = CreateLabel(
    (Rect){pos_x + 15, 0, 0, pos_y + 370}, "是否申请管理员账号？(Y/N)", kBlack, NULL_ID
  );
  InputBox* admin_input = CreateInputBox(
    (Rect){pos_x + 250, pos_x + 350, 0, pos_y + 370}, "", NULL_ID
  );
  Button* confirm_button = CreateButton(
    (Rect){pos_x + 100, pos_x + 300, pos_y + 400, pos_y + 470}, "确认",
    CONFIRM_COLOR, 1, kBlack, 201
  );
  InsertComp(confirm_button, kButton);
  InsertComp(admin_input, kInputBox);
  InsertComp(sex_input, kInputBox);
  InsertComp(dpt_input, kInputBox);
  InsertComp(second_pw_input, kInputBox);
  InsertComp(first_pw_input, kInputBox);
  InsertComp(username_input, kInputBox);
  InsertComp(admin_label, kLabel);
  InsertComp(sex_label, kLabel);
  InsertComp(dpt_label, kLabel);
  InsertComp(second_pw_label, kLabel);
  InsertComp(first_pw_label, kLabel);
  InsertComp(username_label, kLabel);
  InsertComp(register_title, kLabel);
}

void AddUserLogIn() {
  int pos_x = GetWindowWidthPx() / 2 - 200;
  int pos_y = GetWindowHeightPx() / 2 - 100;
  Frame* center_frame = CreateFrame(
    (Rect){pos_x, pos_x + 400, pos_y, pos_y + 250},
    "E0E0E0", 1
  );
  InsertFrame(center_frame);
  Label *register_title = CreateLabel(
    (Rect){pos_x + 5, 0, 0, pos_y + 25}, "用户登陆：", kBlack, NULL_ID
  );
  Label *username_label = CreateLabel(
    (Rect){pos_x + 15, 0, 0, pos_y + 70}, "用户名：", kBlack, NULL_ID
  );
  InputBox* username_input = CreateInputBox(
    (Rect){pos_x + 150, pos_x + 350, 0, pos_y + 70}, "", NULL_ID
  );
  Label *first_pw_label = CreateLabel(
    (Rect){pos_x + 15, 0, 0, pos_y + 130}, "密码：", kBlack, NULL_ID
  );
  InputBox* first_pw_input = CreateInputBox(
    (Rect){pos_x + 150, pos_x + 350, 0, pos_y + 130}, "", NULL_ID
  );
  Button* confirm_button = CreateButton(
    (Rect){pos_x + 100, pos_x + 300, pos_y + 160, pos_y + 220}, "登录",
    CONFIRM_COLOR, 1, kBlack, 401
  );
  InsertComp(confirm_button, kButton);
  InsertComp(first_pw_input, kInputBox);
  InsertComp(username_input, kInputBox);
  InsertComp(first_pw_label, kLabel);
  InsertComp(username_label, kLabel);
  InsertComp(register_title, kLabel);
}

void AddUserModify() {
  /*
   * ID configuration
   * 301: confirm
   */
  int pos_x = GetWindowWidthPx() / 2 - 200;
  int pos_y = GetWindowHeightPx() / 2 - 250;
  Frame* center_frame = CreateFrame(
    (Rect){pos_x, pos_x + 400, pos_y, pos_y + 500},
    "E0E0E0", 1
  );
  InsertFrame(center_frame);
  Label *register_title = CreateLabel(
    (Rect){pos_x + 5, 0, 0, pos_y + 25}, "用户修改：", kBlack, NULL_ID
  );
  Label *username_label = CreateLabel(
    (Rect){pos_x + 15, 0, 0, pos_y + 70}, "用户名：", kBlack, NULL_ID
  );
  InputBox* username_input = CreateInputBox(
    (Rect){pos_x + 150, pos_x + 350, 0, pos_y + 70}, cur_user->username, NULL_ID
  );
  Label *first_pw_label = CreateLabel(
    (Rect){pos_x + 15, 0, 0, pos_y + 130}, "原密码：", kBlack, NULL_ID
  );
  InputBox* first_pw_input = CreateInputBox(
    (Rect){pos_x + 150, pos_x + 350, 0, pos_y + 130}, "", NULL_ID
  );
  Label *second_pw_label = CreateLabel(
    (Rect){pos_x + 15, 0, 0, pos_y + 190}, "新密码：", kBlack, NULL_ID
  );
  InputBox* second_pw_input = CreateInputBox(
    (Rect){pos_x + 150, pos_x + 350, 0, pos_y + 190}, "", NULL_ID
  );
  Label *dpt_label = CreateLabel(
    (Rect){pos_x + 15, 0, 0, pos_y + 250}, "重复新密码：", kBlack, NULL_ID
  );
  InputBox* dpt_input = CreateInputBox(
    (Rect){pos_x + 150, pos_x + 350, 0, pos_y + 250}, "", NULL_ID
  );
  Label *sex_label = CreateLabel(
    (Rect){pos_x + 15, 0, 0, pos_y + 310}, "性别：（真的要改吗）", kBlack, NULL_ID
  );
  InputBox* sex_input = CreateInputBox(
    (Rect){pos_x + 250, pos_x + 350, 0, pos_y + 310},
    cur_user->gender == MALE ? "M" : "F", NULL_ID
  );
  Label *admin_label = CreateLabel(
    (Rect){pos_x + 15, 0, 0, pos_y + 370}, "部门", kBlack, NULL_ID
  );
  InputBox* admin_input = CreateInputBox(
    (Rect){pos_x + 250, pos_x + 350, 0, pos_y + 370}, cur_user->department, NULL_ID
  );
  Button* confirm_button = CreateButton(
    (Rect){pos_x + 100, pos_x + 300, pos_y + 400, pos_y + 470}, "确认",
    CONFIRM_COLOR, 1, kBlack, 301
  );
  InsertComp(confirm_button, kButton);
  InsertComp(admin_input, kInputBox);
  InsertComp(sex_input, kInputBox);
  InsertComp(dpt_input, kInputBox);
  InsertComp(second_pw_input, kInputBox);
  InsertComp(first_pw_input, kInputBox);
  InsertComp(username_input, kInputBox);
  InsertComp(admin_label, kLabel);
  InsertComp(sex_label, kLabel);
  InsertComp(dpt_label, kLabel);
  InsertComp(second_pw_label, kLabel);
  InsertComp(first_pw_label, kLabel);
  InsertComp(username_label, kLabel);
  InsertComp(register_title, kLabel);
}

// TODO 链表！
void AddUserManagement() {
  Frame* left_frame = CreateFrame(
    (Rect){20, GetWindowWidthPx() / 2 - 10, 100, GetWindowHeightPx() - 100},
    PANEL_COLOR, 1
  );
  Frame* right_frame = CreateFrame(
    (Rect){GetWindowWidthPx() / 2 + 10, GetWindowWidthPx() - 10, 100, GetWindowHeightPx() - 100},
    PANEL_COLOR, 1
  );
  InsertFrame(left_frame);
  InsertFrame(right_frame);
  Label* to_be_varified_label = CreateLabel(
    (Rect){25, 0, 0, 150}, "待审核用户：", kBlack, NULL_ID
  );
  Label* user_list_label = CreateLabel(
    (Rect){GetWindowWidthPx() / 2 + 15, 0, 0, 150}, "已存在用户：", kBlack, NULL_ID
  );
  InsertComp(to_be_varified_label, kLabel);
  InsertComp(user_list_label, kLabel);
}

// TODO 链表！
void AddLibrary() {
  Label* title = CreateLabel(
    (Rect){10, 0, 0, 110}, "当前图书库图书：", kBlack, NULL_ID
  );
  Button* sort_by_id = CreateButton(
    (Rect){150, 250, 80, 125}, "按ID排序", SEARCH_COLOR, 1,
    kWhite, 501
  );
  Button* sort_by_title = CreateButton(
    (Rect){260, 360, 80, 125}, "按标题排序", SEARCH_COLOR, 1,
    kWhite, 502
  );
  Button* sort_by_author = CreateButton(
    (Rect){370, 470, 80, 125}, "按作者排序", SEARCH_COLOR, 1,
    kWhite, 503
  );
  InsertComp(sort_by_author, kButton);
  InsertComp(sort_by_title, kButton);
  InsertComp(sort_by_id, kButton);
  InsertComp(title, kLabel);
}

void AddBookDisplay() {
  
}

void AddContents() {
  switch (cur_page) {
    case kLendAndBorrow:
      AddLendAndBorrow();
      break;
    case kBookSearch:
      AddBookSearch();
      break;
    case kUserSearch:
      AddUserSearch();
      break;
    case kManual:
      break;
    case kAbout:
      break;
    case kUserRegister:
      AddUserRegister();
      break;
    case kUserLogIn:
      AddUserLogIn();
      break;
    case kUserModify:
      AddUserModify();
      break;
    case kUserManagement:
      AddUserManagement();
      break;
    case kLibrary:
      AddLibrary();
      break;
    case kBookDisplay:
      AddBookDisplay();
      break;
    case kBookInit:
      //AddBookInit();
      break;
    case kBookModify:
      //AddBookModify();
      break;
    case kBorrowDisplay:
      //AddBorrowDIsplay();
      break;
    case kStatistics:
      //AddStatistics();
      break;
  }
}

void DrawUI(Page page, User* user, void* info, char* terminal) {
  cur_page = page;
  cur_user = user;
  cur_info = info;
  strcpy(cur_terminal, terminal);
  free(terminal);
  InitPage();
}

// Switch to a new page
void InitPage() {
  InitFrame();
  InitSurface();
  InitComponents();
  AddHeadBar();
  AddFooBar();
  AddContents();
  FlushScreen(GetMouseX(), GetMouseY());
}

void InitGUI() {
  InitGraphics();
  InitConsole();
  InitializeUI();
  cur_page = kWelcome;
  cur_user = NULL;
  hb_status = kUnclicked;
  InitPage();
}

// Handle Callback
void CallbackById(int id) {
  if (id < 0) {
    // click on the head bar
    InitSurface();
    AddSubmenu(id);
  }
  printf("%d\n", id);
}