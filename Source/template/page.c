#include "ui.h"
#include "gui.h"
#include "graphics.h"
#include "extgraph.h"

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

void AddSubmenuFile() {
}

void AddSubmenu() {
  switch (hb_status) {
    case kFile:
      AddSubmenuFile();
      break;
    case kBook:
      break;
    case kUser:
      break;
    case kHelp:
      break;
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
  InitializeUI();
  // AddSubmenu();   // Add submenu first to prevent overlap
  AddHeadBar();
  FlushScreen();
}

// Handle Callback
void CallbackById(int id) {
  printf("%d\n", id);
}