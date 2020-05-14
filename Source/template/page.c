#include "ui.h"
#include "gui.h"
#include "graphics.h"
#include "extgraph.h"
#include "page.h"

typedef enum {
  kUnclicked,
  kFile,
  kBook,
  kUser,
  kHelp,
  kSearch
} HeadBarStatus;

HeadBarStatus hb_status;

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

/* Headbar menu */
static Link *HelpButton, *FileButton, *BookButton,
            *LendAndBorrowButton, *UserButton,
            *SearchButton, *Quit, *Statistic;
static Button *return_button;
static Label *bottom_output;

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

void ExitSurface() {
  hb_status = kUnclicked;
  InitSurface();
}

void AddSubmenuFile() {
  init_lib = CreateButton(
    (Rect) {0, 150, 70, 110},
    "新建", 20, MENU_COLOR, 1, kWhite, 1
  );
  open_lib = CreateButton(
    (Rect) {0, 150, 110, 150},
    "打开", 20, MENU_COLOR, 1, kWhite, 2
  );
  save_lib = CreateButton(
    (Rect) {0, 150, 150, 190},
    "保存", 20, MENU_COLOR, 1, kWhite, 3
  );
  quit = CreateButton(
    (Rect) {0, 150, 190, 230},
    "退出", 20, MENU_COLOR, 1, kWhite, 4
  );
  InsertSurface(init_lib, kButton);
  InsertSurface(open_lib, kButton);
  InsertSurface(save_lib, kButton);
  InsertSurface(quit, kButton);
}

void AddSubmenuBook() {
  new_book = CreateButton(
    (Rect) {70, 250, 70, 110},
    "新建", 20, MENU_COLOR, 1, kWhite, 1
  );
  display_book = CreateButton(
    (Rect) {70, 250, 110, 150},
    "显示", 20, MENU_COLOR, 1, kWhite, 2
  );
  InsertSurface(new_book, kButton);
  InsertSurface(display_book, kButton);
}

void AddSubmenuUser() {
  login = CreateButton(
    (Rect) {200, 350, 70, 110},
    "登录", 20, MENU_COLOR, 1, kWhite, 1
  );
  new_user = CreateButton(
    (Rect) {200, 350, 110, 150},
    "新建", 20, MENU_COLOR, 1, kWhite, 2
  );
  varify = CreateButton(
    (Rect) {200, 350, 150, 190},
    "审核", 20, MENU_COLOR, 1, kWhite, 3
  );
  logout = CreateButton(
    (Rect) {200, 350, 190, 230},
    "登出", 20, MENU_COLOR, 1, kWhite, 4
  );
  InsertSurface(login, kButton);
  InsertSurface(new_user, kButton);
  InsertSurface(varify, kButton);
  InsertSurface(logout, kButton);
}

void AddSubmenuHelp() {
  about = CreateButton(
    (Rect) {GetWindowWidthPx() - 150, GetWindowWidthPx(), 70, 110},
    "关于", 20, MENU_COLOR, 1, kWhite, 1
  );
  manual = CreateButton(
    (Rect) {GetWindowWidthPx() - 150, GetWindowWidthPx(), 110, 150},
    "用户手册", 20, MENU_COLOR, 1, kWhite, 2
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
  }
}

void InitConstant() {
  // Head bar
  FileButton = CreateLink(
    (Rect) {15, 75, 5, 45},
    "文件", 20, kWhite, FILE_ID
  );
  BookButton = CreateLink(
    (Rect) {85, 145, 5, 45},
    "图书", 20, kWhite, BOOK_ID
  );
  LendAndBorrowButton = CreateLink(
    (Rect) {155, 215, 5, 45},
    "借还", 20, kWhite, LEND_ID
  );
  UserButton = CreateLink(
    (Rect) {225, 285, 5, 45},
    "用户", 20, kWhite, USER_ID
  );
  SearchButton = CreateLink(
    (Rect) {295, 355, 5, 45},
    "搜索", 20, kWhite, SEARCH_ID
  );
  Statistic = CreateLink(
    (Rect) {365, 425, 5, 45},
    "统计", 20, kWhite, ST_ID
  );
  return_button = CreateButton(
    (Rect) {435, 495, 15, 55},
    "返回", 20, "757575", 1, kWhite, RTN_ID
  );
  HelpButton = CreateLink(
    (Rect) {GetWindowWidthPx() - 60, 0, 5, 45},
    "帮助", 20, kWhite, HELP_ID
  );
  hb_frame = CreateFrame(
    (Rect) {0, GetWindowWidthPx(), 0, 70},
    "2979FF", 0.5
  );

  // foo bar
  bottom_output = CreateLabel (
    (Rect) {5, GetWindowWidthPx(), GetWindowHeightPx() - 50, GetWindowHeightPx() - 15},
    "状态栏输出：", 20, kWhite, NULL_ID
  );
  fb_frame = CreateFrame(
    (Rect) {0, GetWindowWidthPx(), GetWindowHeightPx() - 50, GetWindowHeightPx()},
    "212121", 0.5
  );
}

void AddHeadBar() {
  InitConstant();
  InsertFrame(hb_frame);
  InsertFrame(fb_frame);
  InsertComp(bottom_output, kLabel);
  InsertComp(HelpButton, kLink);
  InsertComp(return_button, kButton);
  InsertComp(Statistic, kLink);
  InsertComp(SearchButton, kLink);
  InsertComp(UserButton, kLink);
  InsertComp(LendAndBorrowButton, kLink);
  InsertComp(BookButton, kLink);
  InsertComp(FileButton, kLink);
}

void InitPage() {
  AddHeadBar();
  FlushScreen(GetMouseX(), GetMouseY());
}

// Handle Callback
void CallbackById(int id) {
  if (id < 0) {
    // click on the head bar
    AddSubmenu(id);
  }
  printf("%d\n", id);
}