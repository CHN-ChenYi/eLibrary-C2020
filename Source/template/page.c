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
#define SEARCH_COLOR "1976D2"
#define PANEL_COLOR "E0E0E0"
#define TURN_COLOR "1976D2"

/* Headbar menu */
static Link *HelpButton, *FileButton, *BookButton,
            *LendAndBorrowButton, *UserButton,
            *SearchButton, *Quit, *Statistic;
static Button *return_button;
static Label *bottom_output, *bottom_info;

/* Background for head bar and foot bar */
static Frame *hb_frame, *fb_frame;

// 数组作用：将callback id和对应的组件结合起来
#define MAX_ON_PAGE 20
static ListNode* books_on_page[MAX_ON_PAGE];
static User *user_on_page[MAX_ON_PAGE];
static ListNode* tbv_user_on_page[MAX_ON_PAGE]; 
static ListNode* v_user_on_page[MAX_ON_PAGE];
static char *keyword_on_page;
static char *username_on_page;
static char *old_pwd_on_page;
static char *pwd_on_page;
static char *rep_pwd_on_page;
static char *gender_on_page;
static char *dpt_on_page;
static char *whoami_on_page;

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
 * 101 用户搜索
 * 102 用户搜索上一页
 * 103 用户搜索下一页
 * 151 - ? 用户详细信息
 * 201 用户注册确认
 * 301 用户修改确认
 * 401 登陆
 * 501 按照ID排序
 * 502 按照title排序
 * 503 按照author排序
 * 504 显示上一页书库
 * 505 显示下一页书库
 * 551 - ? 书籍详细信息
 * 601 书籍搜索
 * 602 书籍搜索上一页
 * 603 书籍搜索下一页
 * 651 - ？ 书籍详细信息
 * 701 用户管理-未审核上一页
 * 702 用户管理-未审核下一页
 * 703 用户管理-已存在上一页
 * 704 用户管理-已存在下一页
 * 740 - ? 用户管理-同意（奇数）/拒绝（偶数）第k个申请
 * 770 - ？用户管理-删除第k
 * 801 确认新建/修改
 * 802 新建/修改图片
 * 803 删除图书按钮
 * 804 借书按钮
 * 901 借还书界面上一页
 * 902 借还书界面下一页
 * 1001 分类统计标签上一页
 * 1002 分类统计标签下一页
 * 1003 分类统计结果上一页
 * 1004 分类统计结果上一页
 */

LendAndBorrow* test() {
  LendAndBorrow* state = malloc(sizeof(LendAndBorrow));
  state->books = NewList();
  state->borrow_records = NewList();

  Book* book_1 = malloc(sizeof(Book));
  strcpy(book_1->authors[0], "author1");
  strcpy(book_1->category, "category1");
  strcpy(book_1->id, "id1");
  strcpy(book_1->press, "press1");
  strcpy(book_1->title, "title_1");
  InsertList(state->books, state->books->dummy_tail, book_1);

  BorrowRecord* record_1 = malloc(sizeof(BorrowRecord));
  strcpy(record_1->book_name, "title_1");
  record_1->book_status = BORROWED;
  strcpy(record_1->borrowed_date, "20200101");
  strcpy(record_1->returned_date, "20201010");
  strcpy(record_1->user_name, "user");
  InsertList(state->borrow_records, state->borrow_records->dummy_tail, record_1);

  state->books_start = state->books->dummy_head->nxt;
  state->borrow_records_start = state->borrow_records->dummy_head->nxt;

  return state;
}

List* GenBook() {
  List* list = NewList();
  for (int i = 0; i < 10; i++) {
    Book* book = malloc(sizeof(Book));
    for (int j = 0; j < 3; j++) sprintf(book->authors[j], "author%d|%d", j, i);
    for (int j = 0; j < 5; j++) sprintf(book->keywords[j], "keyword%d|%d", j, i);
    sprintf(book->category, "category%d", i);
    sprintf(book->id, "id%d", i);
    sprintf(book->press, "press%d", i);
    sprintf(book->title, "title%d", i);
    InsertList(list, list->dummy_tail, book);
  }
  return list;
}
List* GenUser() {
  List* list = NewList();
  for (int i = 0; i < 10; i++) {
    User* user = malloc(sizeof(User));
    sprintf(user->department, "department%d", i);
    user->gender = i % 2 ? MALE : FEMALE;
    user->whoami = i % 2 ? ADMINISTRATOR : NORMAL_USER;
    user->verified = i % 2 ? TRUE : FALSE;
    InsertList(list, list->dummy_tail, user);
  }
  return list;
}
List* GenBorrowRecord() {
  List* list = NewList();
  for (int i = 0; i < 10; i++) {
    BorrowRecord* record = malloc(sizeof(BorrowRecord));
    sprintf(record->book_name, "title%d", i);
    record->book_status = i % 2 ? BORROWED : RETURNED;
    sprintf(record->borrowed_date, "202001%02d", i);
    sprintf(record->returned_date, "202010%02d", i);
    sprintf(record->user_name, "user%d", i);
    sprintf(record->book_name, "book%d", i);
    InsertList(list, list->dummy_tail, record);
  }
  return list;
}

/*Statistics* test() {
  Statistics* state = malloc(sizeof(Statistics));
  state->catalogs = NewList();
  state->borrow_record = NewList();

  InsertList(state->catalogs, state->catalogs->dummy_tail, "math");
  InsertList(state->catalogs, state->catalogs->dummy_tail, "physics");

  BorrowRecord* record_1 = malloc(sizeof(BorrowRecord));
  strcpy(record_1->book_name, "title_1");
  record_1->book_status = BORROWED;
  strcpy(record_1->borrowed_date, "20200101");
  strcpy(record_1->returned_date, "20201010");
  strcpy(record_1->user_name, "user_1");
  InsertList(state->borrow_record, state->borrow_record->dummy_tail, record_1);

  state->catalogs_start = state->catalogs->dummy_head->nxt;
  state->borrow_record_start = state->borrow_record->dummy_head->nxt;

  return state;
}*/


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

// TODO(Hans)：链表！
void AddLendAndBorrow() {
  /*
   * ID configuration
   * 101: search book
   */
  int left_border = 30;
  int length_input_box = 120;
  int search_button_top = GetWindowHeightPx() - 120;
  int search_button_bottom = GetWindowHeightPx() - 100;
  Label *return_title = CreateLabel(
    (Rect){left_border, 0, 0, 100}, "还书：", kBlack, NULL_ID
  );
  Label *borrow_title = CreateLabel(
    (Rect){left_border, 100, search_button_top, search_button_bottom},
    "借书：", kBlack, NULL_ID
  );
  InputBox *input_box = CreateInputBox(
    (Rect){left_border + TextStringWidth("借书："),
           length_input_box + left_border + TextStringWidth("借书："),
           search_button_top, search_button_bottom},
    "", NULL_ID
  );
  Button *search_button = CreateButton(
    (Rect){length_input_box + left_border + TextStringWidth("借书：") + 10,
           length_input_box + left_border + TextStringWidth("借书：") + 80,
           search_button_top - 5, search_button_bottom + 10}, 
    "搜索", SEARCH_COLOR, 1, kWhite, 101
  );
  InsertComp(search_button, kButton);
  InsertComp(input_box, kInputBox);
  InsertComp(return_title, kLabel);
  InsertComp(borrow_title, kLabel);
}

// TODO(Hans)：链表！借书按钮不要忘记记录书本数组
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
  keyword_on_page = input_box->context;
  Button *button = CreateButton(
    (Rect){400, 500, 120, 160}, "搜索", SEARCH_COLOR, 1, kBlack, 601
  );
  Button *pre_page_button = CreateButton(
    (Rect){20, 100, GetWindowHeightPx() - 125, GetWindowHeightPx() - 75},
    "上一页", TURN_COLOR, 1, kWhite, 602
  );
  Button *next_page_button = CreateButton(
    (Rect){GetWindowWidthPx() - 100, GetWindowWidthPx() - 20, GetWindowHeightPx() - 125, GetWindowHeightPx() - 75},
    "下一页", TURN_COLOR, 1, kWhite, 603
  );
  InsertComp(next_page_button, kButton);
  InsertComp(pre_page_button, kButton);
  InsertComp(button, kButton);
  InsertComp(search_title, kLabel);
  InsertComp(search_info, kLabel);
  InsertComp(input_box, kInputBox);
}

/*
 * 601 书籍搜索
 * 602 书籍搜索上一页
 * 603 书籍搜索下一页
 * 651 - ? 借书
 */
void HandleBookSearchCallback(int id) {
  State cur_state;
  cur_state.book_search = cur_info;
  switch (id) {
  case 1:
    cur_state.book_search->search_callback(keyword_on_page);
    break;
  case 2:
    cur_state.book_search->turn_page(0);
    break;
  case 3:
    cur_state.book_search->turn_page(1);
    break;
  default:
    cur_state.book_search->borrow_callback(books_on_page[id - 50]);
    break;
  }
}

// TODO(Hans)：链表！
// 注意在加用户的时候把user_on_page设置一下！
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
  keyword_on_page = input_box->context;
  Button *search_button = CreateButton(
    (Rect){400, 500, 120, 160}, "搜索", SEARCH_COLOR, 1, kBlack, 101
  );
  Button *pre_page_button = CreateButton(
    (Rect){20, 100, GetWindowHeightPx() - 125, GetWindowHeightPx() - 75},
    "上一页", TURN_COLOR, 1, kWhite, 102
  );
  Button *next_page_button = CreateButton(
    (Rect){GetWindowWidthPx() - 100, GetWindowWidthPx() - 20, GetWindowHeightPx() - 125, GetWindowHeightPx() - 75},
    "下一页", TURN_COLOR, 1, kWhite, 103
  );
  InsertComp(next_page_button, kButton);
  InsertComp(pre_page_button, kButton);
  InsertComp(next_page_button, kButton);
  InsertComp(pre_page_button, kButton);
  InsertComp(search_button, kButton);
  InsertComp(search_title, kLabel);
  InsertComp(search_info, kLabel);
  InsertComp(input_box, kInputBox);
}

/*
 * 1 用户搜索
 * 2 用户搜索上一页
 * 3 用户搜索下一页
 * 51 - ? 用户详细信息
 */
void HandleUserSearchCallback(int id){
  State cur_state;
  cur_state.user_search = cur_info;
  switch (id) {
  case 1:
    cur_state.user_search->search_callback(keyword_on_page);
    break;
  case 2:
    cur_state.user_search->turn_page(0);
    break;
  case 3:
    cur_state.user_search->turn_page(1);
    break;
  default:
    cur_state.user_search->info_callback(user_on_page[id - 50]);
    break;
  }
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
  username_on_page = username_input->context;
  Label *first_pw_label = CreateLabel(
    (Rect){pos_x + 15, 0, 0, pos_y + 130}, "密码：", kBlack, NULL_ID
  );
  InputBox* first_pw_input = CreateInputBox(
    (Rect){pos_x + 150, pos_x + 350, 0, pos_y + 130}, "", NULL_ID
  );
  pwd_on_page = first_pw_input->context;
  Label *second_pw_label = CreateLabel(
    (Rect){pos_x + 15, 0, 0, pos_y + 190}, "重复密码：", kBlack, NULL_ID
  );
  InputBox* second_pw_input = CreateInputBox(
    (Rect){pos_x + 150, pos_x + 350, 0, pos_y + 190}, "", NULL_ID
  );
  rep_pwd_on_page = second_pw_input->context;
  Label *dpt_label = CreateLabel(
    (Rect){pos_x + 15, 0, 0, pos_y + 250}, "部门：", kBlack, NULL_ID
  );
  InputBox* dpt_input = CreateInputBox(
    (Rect){pos_x + 150, pos_x + 350, 0, pos_y + 250}, "", NULL_ID
  );
  dpt_on_page = dpt_input->context;
  Label *sex_label = CreateLabel(
    (Rect){pos_x + 15, 0, 0, pos_y + 310}, "性别（M/F）", kBlack, NULL_ID
  );
  InputBox* sex_input = CreateInputBox(
    (Rect){pos_x + 150, pos_x + 350, 0, pos_y + 310}, "", NULL_ID
  );
  gender_on_page = sex_input->context;
  Label *admin_label = CreateLabel(
    (Rect){pos_x + 15, 0, 0, pos_y + 370}, "是否申请管理员账号？(Y/N)", kBlack, NULL_ID
  );
  InputBox* admin_input = CreateInputBox(
    (Rect){pos_x + 250, pos_x + 350, 0, pos_y + 370}, "", NULL_ID
  );
  whoami_on_page = admin_input->context;
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

void HandleUserRegisterCallback(int id) {
  State cur_state;
  cur_state.login_or_register = cur_info;
  switch (id) {
  case 1:
    strcpy(cur_state.login_or_register->password, pwd_on_page);
    strcpy(cur_state.login_or_register->repeat_password, rep_pwd_on_page);
    strcpy(cur_state.login_or_register->user->username, username_on_page);
    strcpy(cur_state.login_or_register->user->department, dpt_on_page);
    if (strcmp(gender_on_page, "F") == 0) {
      cur_state.login_or_register->user->gender = FEMALE;
    } else {
      // 不合法性别默认为男性
      cur_state.login_or_register->user->gender = MALE;
    }
    if (strcmp(whoami_on_page, "Y") == 0) {
      cur_state.login_or_register->user->whoami = ADMINISTRATOR;
    } else {
      // 不合法默认不申请
      cur_state.login_or_register->user->whoami = NORMAL_USER;
    }
    cur_state.login_or_register->login_callback();
    break;
  }
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
  username_on_page = username_input->context;
  Label *first_pw_label = CreateLabel(
    (Rect){pos_x + 15, 0, 0, pos_y + 130}, "密码：", kBlack, NULL_ID
  );
  InputBox* first_pw_input = CreateInputBox(
    (Rect){pos_x + 150, pos_x + 350, 0, pos_y + 130}, "", NULL_ID
  );
  pwd_on_page = first_pw_input->context;
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

void HandleUserLoginCallback(int id) {
  State cur_state;
  cur_state.login_or_register = cur_info;
  switch (id) {
  case 1:
    strcpy(cur_state.login_or_register->password, pwd_on_page);
    strcpy(cur_state.login_or_register->user->username, username_on_page);
    cur_state.login_or_register->login_callback();
    break;
  }
}

// TODO: 分成两栏！日了狗了
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
  username_on_page = username_input->context;
  Label *first_pw_label = CreateLabel(
    (Rect){pos_x + 15, 0, 0, pos_y + 130}, "原密码：", kBlack, NULL_ID
  );
  InputBox* first_pw_input = CreateInputBox(
    (Rect){pos_x + 150, pos_x + 350, 0, pos_y + 130}, "", NULL_ID
  );
  old_pwd_on_page = first_pw_input->context;
  Label *second_pw_label = CreateLabel(
    (Rect){pos_x + 15, 0, 0, pos_y + 190}, "新密码：", kBlack, NULL_ID
  );
  InputBox* second_pw_input = CreateInputBox(
    (Rect){pos_x + 150, pos_x + 350, 0, pos_y + 190}, "", NULL_ID
  );
  pwd_on_page = second_pw_input->context;
  Label *dpt_label = CreateLabel(
    (Rect){pos_x + 15, 0, 0, pos_y + 250}, "重复新密码：", kBlack, NULL_ID
  );
  InputBox* dpt_input = CreateInputBox(
    (Rect){pos_x + 150, pos_x + 350, 0, pos_y + 250}, "", NULL_ID
  );
  rep_pwd_on_page = dpt_input->context;
  Label *sex_label = CreateLabel(
    (Rect){pos_x + 15, 0, 0, pos_y + 310}, "性别：（真的要改吗）", kBlack, NULL_ID
  );
  InputBox* sex_input = CreateInputBox(
    (Rect){pos_x + 250, pos_x + 350, 0, pos_y + 310},
    cur_user->gender == MALE ? "M" : "F", NULL_ID
  );
  gender_on_page = sex_input->context;
  Label *admin_label = CreateLabel(
    (Rect){pos_x + 15, 0, 0, pos_y + 370}, "部门", kBlack, NULL_ID
  );
  InputBox* admin_input = CreateInputBox(
    (Rect){pos_x + 250, pos_x + 350, 0, pos_y + 370}, cur_user->department, NULL_ID
  );
  dpt_on_page = admin_input->context;
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

void HandleUserModifyCallback(int id) {
  State cur_state;
  cur_state.user_modify = cur_info;
  switch (id) {
  case 1:
    strcpy(cur_state.user_modify->old_password, old_pwd_on_page);
    strcpy(cur_state.user_modify->new_password, pwd_on_page);
    strcpy(cur_state.user_modify->repeat_password, rep_pwd_on_page);
    strcpy(cur_state.user_modify->user->department, dpt_on_page);
    strcpy(cur_state.user_modify->user->username, username_on_page);
    if (strcmp(gender_on_page, "F") == 0) {
      cur_state.user_modify->user->gender = FEMALE;
    }
    else {
      cur_state.user_modify->user->gender = MALE;
      // 不合法性别默认为男性
    }
    cur_state.user_modify->confirm_callback();
    break;
  }
}

// TODO 链表！两个链表都需要记录on page信息！
// 分别是tbv和v
void AddUserManagement() {
  int left_border = 20;
  int right_border = GetWindowWidthPx() - 20;
  int middle = GetWindowWidthPx() / 2;
  int delta_y = 40;
  int left_x = left_border + 10, left_cur_y = 100;
  int right_x = middle + 20, right_cur_y = 100;
  int bottom = GetWindowHeightPx() - 100;
  Frame* left_frame = CreateFrame(
    (Rect){left_x - 10, middle - 10, left_cur_y, bottom},
    PANEL_COLOR, 1
  );
  Frame* right_frame = CreateFrame(
    (Rect){middle + 10, right_border, right_cur_y, bottom},
    PANEL_COLOR, 1
  );
  InsertFrame(left_frame);
  InsertFrame(right_frame);
  Label* to_be_varified_label = CreateLabel(
    (Rect){left_x, 0, 0, left_cur_y += delta_y}, "待审核用户：", kBlack, NULL_ID
  );

  Label* user_list_label = CreateLabel(
    (Rect){right_x, 0, 0, right_cur_y += delta_y}, "已存在用户：", kBlack, NULL_ID
  );
  
  Button *pre_page_left_button = CreateButton(
    (Rect){left_border, left_border + 80, bottom - 25, bottom + 25},
    "上一页", TURN_COLOR, 1, kWhite, 701
  );
  Button *next_page_left_button = CreateButton(
    (Rect){middle - 90, middle - 10, bottom - 25, bottom + 25},
    "下一页", TURN_COLOR, 1, kWhite, 702
  );
  Button *pre_page_right_button = CreateButton(
    (Rect){middle + 10, middle + 90, bottom - 25, bottom + 25},
    "上一页", TURN_COLOR, 1, kWhite, 703
  );
  Button *next_page_right_button = CreateButton(
    (Rect){right_border - 80, right_border, bottom - 25, bottom + 25},
    "下一页", TURN_COLOR, 1, kWhite, 704
  );
  InsertComp(next_page_right_button, kButton);
  InsertComp(pre_page_right_button, kButton);
  InsertComp(next_page_left_button, kButton);
  InsertComp(pre_page_left_button, kButton);
  InsertComp(to_be_varified_label, kLabel);
  InsertComp(user_list_label, kLabel);
}

/*
 * 701 用户管理 - 未审核上一页
 * 702 用户管理 - 未审核下一页
 * 703 用户管理 - 已存在上一页
 * 704 用户管理 - 已存在下一页
 * 740 - ? 用户管理-同意（奇数）/拒绝（偶数）第k个申请
 * 770 - ？用户管理-删除第k
 */
void HandleUserManagement(int id) {
  State cur_state;
  cur_state.user_management = cur_info;
  switch (id) {
  case 1:
    cur_state.user_management->turn_page(0, 0);
    break;
  case 2:
    cur_state.user_management->turn_page(1, 0);
    break;
  case 3:
    cur_state.user_management->turn_page(0, 1);
    break;
  case 4:
    cur_state.user_management->turn_page(1, 1);
    break;
  default:
    if(id > 70) {
      cur_state.user_management->delete_callback(v_user_on_page[id - 70]);
    } else {
      cur_state.user_management->approve_callback(
        tbv_user_on_page[(id - 40) + 1 >> 1], id & 1
      );
    }
  }
}

// TODO 链表！记得把书籍详细信息记录一下
void AddLibrary() {
  int left_border = 10;
  int right_border = GetWindowWidthPx() - 10;
  int top = 130;
  int bottom = GetWindowHeightPx() - 70;

  Frame* frame = CreateFrame(
    (Rect){left_border, right_border, top, bottom}, PANEL_COLOR, 1
  );
  InsertFrame(frame);

  int cur_y = 110;
  int delta_y = 50;
  int cur_x = 20; // for the head line
  int width_button = 100;
  Label* title = CreateLabel(
    (Rect){cur_x, 0, 0, cur_y}, "当前图书库图书：", kBlack, NULL_ID
  );
  Button* sort_by_id = CreateButton(
    (Rect){cur_x += TextStringWidth("当前图书库图书："), cur_x + 100, 80, 125}, "按ID排序", SEARCH_COLOR, 1,
    kWhite, 501
  );
  Button* sort_by_title = CreateButton(
    (Rect){cur_x += 110, cur_x + 100, 80, 125}, "按标题排序", SEARCH_COLOR, 1,
    kWhite, 502
  );
  Button* sort_by_author = CreateButton(
    (Rect){cur_x += 110, cur_x + 100, 80, 125}, "按作者排序", SEARCH_COLOR, 1,
    kWhite, 503
  );
  Button *pre_page_button = CreateButton(
    (Rect){20, 100, GetWindowHeightPx() - 125, GetWindowHeightPx() - 75},
    "上一页", TURN_COLOR, 1, kWhite, 504
  );
  Button *next_page_button = CreateButton(
    (Rect){GetWindowWidthPx() - 100, GetWindowWidthPx() - 20, GetWindowHeightPx() - 125, GetWindowHeightPx() - 75},
    "下一页", TURN_COLOR, 1, kWhite, 505
  );
  cur_x = 20;
  InsertComp(next_page_button, kButton);
  InsertComp(pre_page_button, kButton);
  InsertComp(sort_by_author, kButton);
  InsertComp(sort_by_title, kButton);
  InsertComp(sort_by_id, kButton);
  InsertComp(title, kLabel);
}

/*
 * 501 按照ID排序
 * 502 按照title排序
 * 503 按照author排序
 * 504 显示上一页书库
 * 505 显示下一页书库
 * 551 - ? 书籍详细信息
 */
void HandleLibraryCallback(int id) {
  State cur_state;
  cur_state.library = cur_info;
  switch (id) {
  case 1:
    cur_state.library->sort_callback(kId);
    break;
  case 2:
    cur_state.library->sort_callback(kTitle);
    break;
  case 3:
    cur_state.library->sort_callback(kAuthor);
    break;
  case 4:
    cur_state.library->turn_page(0);
    break;
  case 5:
    cur_state.library->turn_page(1);
    break;
  default:
    cur_state.library->book_callback(books_on_page[id - 50]);
    break;
  }
}

void AddBookDisplay() {
  // State cur_state;
  // cur_state.book_display = cur_info;
  State cur_state;
  cur_state.book_display = cur_info;
  int info_x = GetWindowWidthPx() / 2;
  int cur_y = 250;
  int delta_y = 30;
  LibImage img = cur_state.book_display->book_cover;
  Image* book_cover = CreateImage(
    (Rect){GetWindowWidthPx() / 4 - img.width / 2, 0, 0, GetWindowHeightPx() - img.height / 2},
    img, NULL_ID
  );
  InsertComp(book_cover, kImage);
  Label* title = CreateLabel(
    (Rect){GetWindowWidthPx() / 4 - img.width / 2, 0, 0, cur_y - 20}, "图书信息", kBlack, NULL_ID
  );
  Label* book_id_label = CreateLabel(
    (Rect){info_x, 0, 0, cur_y}, "书号：", kBlack, NULL_ID
  );
  Label* book_id_context = CreateLabel(
    (Rect){info_x + TextStringWidth("书号："), 0, 0, cur_y},
    cur_state.book_display->book->id, kBlack, NULL_ID
  );
  Label* book_name_label = CreateLabel(
    (Rect){info_x, 0, 0, cur_y += delta_y}, "书名：", kBlack, NULL_ID
  );
  Label* book_name_context = CreateLabel(
    (Rect){info_x + TextStringWidth("书名："), 0, 0, cur_y},
    cur_state.book_display->book->title, kBlack, NULL_ID
  );
  Label* author_label = CreateLabel(
    (Rect){info_x, 0, 0, cur_y += delta_y}, "作者：", kBlack, NULL_ID 
  );
  Label* author_context[3];
  for (int i = 0; i < 3; i++) {
    author_context[i] = CreateLabel(
      (Rect){info_x + TextStringWidth("作者："), 0, 0, cur_y += delta_y},
      cur_state.book_display->book->authors[i], kBlack, NULL_ID
    );
    InsertComp(author_context[i], kLabel);
  }
  Label* press_label = CreateLabel(
    (Rect){info_x, 0, 0, cur_y += delta_y}, "出版社：", kBlack, NULL_ID
  );
  Label* press_context = CreateLabel(
    (Rect){info_x + TextStringWidth("出版社："), 0, 0, cur_y},
    cur_state.book_display->book->press, kBlack, NULL_ID
  );
  Label* keyword_label = CreateLabel(
    (Rect){info_x, 0, 0, cur_y += delta_y}, "关键词：", kBlack, NULL_ID
  );
  Label* keyword_context[5];
  for (int i = 0; i < 5; i++) {
    keyword_context[i] = CreateLabel(
      (Rect){info_x + TextStringWidth("关键词："), 0, 0, cur_y += delta_y},
      cur_state.book_display->book->keywords[i], kBlack, NULL_ID
    );
    InsertComp(keyword_context[i], kLabel);
  }
  InsertComp(title, kLabel);
  InsertComp(book_id_label, kLabel);
  InsertComp(book_id_context, kLabel);
  InsertComp(book_name_label, kLabel);
  InsertComp(book_name_context, kLabel);
  InsertComp(author_label, kLabel);
  InsertComp(press_label, kLabel);
  InsertComp(press_context, kLabel);
  InsertComp(keyword_label, kLabel);
}

/*
 * 801 确认新建 / 修改
 * 802 新建 / 修改图片
 * 803 删除图书按钮
 * 804 借书按钮
 */
void AddBookModify() {
  // State cur_state;
  // cur_state.book_display = cur_info;
  State cur_state;
  cur_state.book_display = cur_info;
  int info_x = GetWindowWidthPx() / 2;
  int right_border = GetWindowWidthPx() - 100;
  int cur_y = 250;
  int delta_y = 30;
  LibImage img = cur_state.book_display->book_cover;
  Book *book = cur_state.book_display->book;
  Image* book_cover = CreateImage(
    (Rect){GetWindowWidthPx() / 4 - img.width / 2, 0, 0, GetWindowHeightPx() - img.height / 2},
    img, 802
  );
  InsertComp(book_cover, kImage);

  Label* title;
  if (cur_page == kBookInit) {
    title = CreateLabel(
      (Rect){GetWindowWidthPx() / 4 - img.width / 2, 0, 0, cur_y - 20}, "图书新建：", kBlack, NULL_ID
    );
  } else {
    title = CreateLabel(
      (Rect){GetWindowWidthPx() / 4 - img.width / 2, 0, 0, cur_y - 20}, "图书修改：", kBlack, NULL_ID
    );
  }
  Label* book_id_label = CreateLabel(
    (Rect){info_x, 0, 0, cur_y}, "书号：", kBlack, NULL_ID
  );
  InputBox* book_id_context = CreateInputBox(
    (Rect){info_x + TextStringWidth("书号："), right_border, 0, cur_y},
    book->id, NULL_ID
  );
  Label* book_name_label = CreateLabel(
    (Rect){info_x, 0, 0, cur_y += delta_y}, "书名：", kBlack, NULL_ID
  );
  InputBox* book_name_context = CreateInputBox(
    (Rect){info_x + TextStringWidth("书名："), right_border, 0, cur_y},
    book->title, NULL_ID
  );
  Label* author_label = CreateLabel(
    (Rect){info_x, 0, 0, cur_y += delta_y}, "作者：（最多三人）", kBlack, NULL_ID 
  );
  InputBox* author_context[3];
  for (int i = 0; i < 3; i++) {
    author_context[i] = CreateInputBox(
      (Rect){info_x + TextStringWidth("作者："), right_border, 0, cur_y += delta_y},
      book->authors[i], NULL_ID
    );
    InsertComp(author_context[i], kInputBox);
  }
  Label* press_label = CreateLabel(
    (Rect){info_x, 0, 0, cur_y += delta_y}, "出版社：", kBlack, NULL_ID
  );
  InputBox* press_context = CreateInputBox(
    (Rect){info_x + TextStringWidth("出版社："), right_border, 0, cur_y},
    book->press, NULL_ID
  );
  Label* keyword_label = CreateLabel(
    (Rect){info_x, 0, 0, cur_y += delta_y}, "关键词：（最多五个）", kBlack, NULL_ID
  );
  InputBox* keyword_context[5];
  for (int i = 0; i < 5; i++) {
    keyword_context[i] = CreateInputBox(
      (Rect){info_x + TextStringWidth("关键词："), right_border, 100, cur_y += delta_y},
      book->keywords[i], NULL_ID
    );
    InsertComp(keyword_context[i], kInputBox);
  }
  Button* confirm_button = CreateButton(
    (Rect){GetWindowWidthPx() - 100, GetWindowWidthPx() - 20, GetWindowHeightPx() - 120, GetWindowHeightPx() - 70},
    "确认", CONFIRM_COLOR, 1, kBlack, 801
  );
  InsertComp(title, kLabel);
  InsertComp(book_id_label, kLabel);
  InsertComp(book_id_context, kInputBox);
  InsertComp(book_name_label, kLabel);
  InsertComp(book_name_context, kInputBox);
  InsertComp(author_label, kLabel);
  InsertComp(press_label, kLabel);
  InsertComp(press_context, kInputBox);
  InsertComp(keyword_label, kLabel);
}

// TODO 链表！
void AddBorrowDisplay() {
  int left_border = 100;
  int cur_y = 200;
  int delta_y = 100;
  Label *title = CreateLabel(
    (Rect) {left_border, 0, 0, 100}, "借还书记录：", kBlack, NULL_ID
  );
  Button *pre_page_button = CreateButton(
    (Rect){20, 100, GetWindowHeightPx() - 125, GetWindowHeightPx() - 75},
    "上一页", TURN_COLOR, 1, kWhite, 901
  );
  Button *next_page_button = CreateButton(
    (Rect){GetWindowWidthPx() - 100, GetWindowWidthPx() - 20, GetWindowHeightPx() - 125, GetWindowHeightPx() - 75},
    "下一页", TURN_COLOR, 1, kWhite, 902
  );
  InsertComp(next_page_button, kButton);
  InsertComp(pre_page_button, kButton);
  InsertComp(title, kLabel);
}

// TODO 链表！
void AddStatistics() {
  int left_border = 100;
  int right_border = GetWindowWidthPx() - 100;
  int middle = GetWindowHeightPx() / 2 - 100;
  int top = 100;
  int bottom = GetWindowHeightPx() - 70;
  Label *title = CreateLabel(
    (Rect) {left_border + 10, 0, 0, top + GetPointSize() + 10}, "分类统计：", kBlack, NULL_ID
  );
  InsertComp(title, kLabel);
  Frame *top_frame = CreateFrame(
    (Rect){left_border, right_border, top, middle - 5}, PANEL_COLOR, 1
  );
  Frame *bottom_frame = CreateFrame(
    (Rect){left_border, right_border, middle + 5, bottom}, PANEL_COLOR, 1
  );
  InsertFrame(top_frame);
  InsertFrame(bottom_frame);
  Button *top_pre_page_button = CreateButton(
    (Rect){left_border + 10, left_border + 110, middle - 60, middle - 10},
    "上一页", SEARCH_COLOR, 1, kWhite, 1001
  );
  Button *top_next_page_button = CreateButton(
    (Rect){right_border - 110, right_border - 10, middle - 60, middle - 10},
    "下一页", SEARCH_COLOR, 1, kWhite, 1002
  );
  Button *bottom_pre_page_button = CreateButton(
    (Rect){left_border + 10, left_border + 110, bottom - 60, bottom - 10},
    "上一页", SEARCH_COLOR, 1, kWhite, 1003
  );
  Button *bottom_next_page_button = CreateButton(
    (Rect){right_border - 110, right_border - 10, bottom - 60, bottom - 10},
    "下一页", SEARCH_COLOR, 1, kWhite, 1004
  );
  InsertComp(bottom_next_page_button, kButton);
  InsertComp(bottom_pre_page_button, kButton);
  InsertComp(top_next_page_button, kButton);
  InsertComp(top_pre_page_button, kButton);
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
    case kBookModify:
    case kBookInit:
      AddBookModify();
      break;
    case kBorrowDisplay:
      AddBorrowDisplay();
      break;
    case kStatistics:
      AddStatistics();
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
  } else {
    switch (id / 100) {
    case 1:
      HandleUserSearchCallback(id % 100);
      break;
    case 2:
      HandleUserRegisterCallback(id % 100);
      break;
    case 3:
      HandleUserModifyCallback(id % 100);
      break;
    case 4:
      HandleUserLoginCallback(id % 100);
    case 5:
      HandleLibraryCallback(id % 100);
      break;
    case 6:
      HandleBookSearchCallback(id % 100);
      break;
    case 7:
      HandleUserManagementCallback(id % 100);
      break;
    }
  }
  printf("%d\n", id);
}