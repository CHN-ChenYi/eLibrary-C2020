#include "gui.h"
#include "page.h"
#include "ui.h"
#include "graphics.h"
#include "extgraph.h"
#include <wingdi.h>
#include <Windows.h>
#include <string.h>
#include <ctype.h>

#define FONT_BUTTON "Source Han Sans"
#define FONT_INPUT "Source Han Sans"
#define FONT_LABEL "Source Han Sans"
#define FONT_LINK "Source Han Sans"

/* Handling of colors */

// Convert a char to a decimal number using scalar initializer
const int C2D[128] = {
    ['0'] = 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
    ['A'] = 10, 11, 12, 13, 14, 15,
    ['a'] = 10, 11, 12, 13, 14, 15
};

// convert a CSS color string into color
Color ColorConvert(char* color, double alpha) {
  return (Color){((C2D[color[0]] << 4) | C2D[color[1]]) << 8,
                 ((C2D[color[2]] << 4) | C2D[color[3]]) << 8,
                 ((C2D[color[4]] << 4) | C2D[color[5]]) << 8,
                 (int)alpha * 65535};
}
/* End of color handling */

/* Singly linked circular list for components */

static PTCNode focus = NULL;      // The current focus
static CompList cur_list = NULL;  // The list of all the components on this page.
static CompList frame = NULL;     // The background or framework of this page.
static CompList surface = NULL;   // The outmost content

// Add a new component into the list
void InsertComp(void* component, TypeOfComp type) {
  PTCNode new_node = (PTCNode)malloc(sizeof(struct ComponentListNode));
  new_node->component = component;
  new_node->type = type;
  new_node->next = cur_list->next;
  cur_list->next = new_node;
}

// Add a new framework into the list
void InsertFrame(void* component) {
  PTCNode new_node = (PTCNode)malloc(sizeof(struct ComponentListNode));
  new_node->component = component;
  new_node->next = frame->next;
  frame->next = new_node;
}

void InsertSurface(void* component, TypeOfComp type) {
  PTCNode new_node = (PTCNode)malloc(sizeof(struct ComponentListNode));
  new_node->component = component;
  new_node->type = type;
  new_node->next = surface->next;
  surface->next = new_node;
}

// Make a new list
CompList NewCompList() {
  CompList new_list = (CompList)malloc(sizeof(struct ComponentListNode));
  new_list->component = NULL;
  new_list->next = new_list;
  new_list->type = kButton;  // terminate in search of label
  return new_list;
}

// Free the list
void FreeList(CompList L) {
  for (PTCNode p = L->next; p != L;) {
    //printf("%d\n", p->type);
    PTCNode next = p->next;
    free(p->component);
    free(p);
    p = next;
  }
  free(L->component);
  free(L);
}

/* End of linked list */

/* Create components */
Button* CreateButton (
    Rect rect,
    char* caption,
    int font_size,
    char* bg_color,
    double alpha,
    FontColor color,
    int id) {
  Button* ret = malloc(sizeof(Button));
  ret->id = id;
  ret->position = rect;
  ret->status = kNormal;
  ret->bg_color = ColorConvert(bg_color, alpha);
  ret->font_color = color;
  ret->font_size = font_size;
  strcpy(ret->caption, caption);
  return ret;
}

InputBox* CreateInputBox(Rect rect, int font_size, int id) {
  InputBox* ret = malloc(sizeof(InputBox));
  ret->id = id;
  ret->position = rect;
  SetFont(FONT_INPUT);
  SetPointSize(font_size);
  ret->position.top = ret->position.bottom - GetPointSize();
  ret->position.top -= 5;
  ret->position.bottom += 5;
  ret->position.left -= 5;
  ret->position.right += 5;
  ret->cursor = 0;
  ret->status = kNormal;
  ret->font_size = font_size;
  memset(ret->context, 0, sizeof(ret->context));
  return ret;
}

Link* CreateLink(Rect rect, char* caption, int font_size, FontColor font_color, int id) {
  Link* ret = malloc(sizeof(Link));
  ret->id = id;
  ret->position = rect;
  SetFont(FONT_LINK);
  SetPointSize(font_size);
  ret->position.right = ret->position.left + TextStringWidth(caption);
  ret->position.top = ret->position.bottom - GetPointSize();
  ret->status = kNormal;
  ret->font_size = font_size;
  ret->font_color = font_color;
  strcpy(ret->caption, caption);
  return ret;
}

Label* CreateLabel(Rect rect, char* caption, int font_size, FontColor font_color, int id) {
  Label* ret = malloc(sizeof(Label));
  ret->id = id;
  ret->position = rect;
  SetFont(FONT_LABEL);
  SetPointSize(font_size);
  ret->position.right = ret->position.left + TextStringWidth(caption);
  ret->position.top = ret->position.bottom - GetPointSize();
  ret->font_size = font_size;
  ret->font_color = font_color;
  strcpy(ret->caption, caption);
  return ret;
}

// Create a color rectangle, color must be CSS-styled
Frame* CreateFrame(Rect rect, char* color, double alpha) {
  Frame* ret = malloc(sizeof(Frame));
  ret->color = ColorConvert(color, alpha);
  ret->position = rect;
  return ret;
}

/* Draw components */

void DrawRectangle(Rect* rect) {
  MovePen(rect->left, rect->top);
  DrawLine(rect->right - rect->left, 0);
  DrawLine(0, rect->bottom - rect->top);
  DrawLine(rect->left - rect->right, 0);
  DrawLine(0, rect->top - rect->bottom);
}

void DrawFrame(Frame* rect) {
  ColorPoint lower_right = (ColorPoint){rect->position.right, rect->position.bottom, rect->color};
  ColorPoint upper_left = (ColorPoint){rect->position.left, rect->position.top, rect->color};
  DrawShadedRectangle(&lower_right, &upper_left);
}

/* Draw a rectangle to wrap it*/
void DrawOuter(Rect* rect) {
  SetPenColor("gray");
  SetPenSize(1);
  MovePen(rect->left - 5, rect->top - 5);
  DrawLine(rect->right - rect->left + 10, 0);
  DrawLine(0, rect->bottom - rect->top + 10);
  DrawLine(rect->left - rect->right - 10, 0);
  DrawLine(0, rect->top - rect->bottom - 10);
}

void DrawButton(Button* button, int mouse_x) {
  SetFont(FONT_BUTTON);
  SetPointSize(button->font_size);
  SetPenSize(1);
  Color bg_color = button->bg_color;
  Color white = ColorConvert("E3F2FD", 1);
  ColorPoint upper_left = (ColorPoint){button->position.left, button->position.top, bg_color};
  ColorPoint lower_right = (ColorPoint){button->position.right, button->position.bottom, bg_color};
  if (button->status == kHover) {
    ColorPoint upper_middle = (ColorPoint){mouse_x, button->position.top, white};
    ColorPoint lower_middle = (ColorPoint){mouse_x, button->position.bottom, white};
    DrawShadedRectangle(&lower_middle, &upper_left);
    DrawShadedRectangle(&lower_right, &upper_middle);
  } else {
    DrawShadedRectangle(&lower_right, &upper_left); 
  }
  // Draw the caption string in the center
  int middle_x = (button->position.left + button->position.right
                  - TextStringWidth(button->caption)) >> 1;
  int middle_y = (button->position.top + button->position.bottom
                  + GetFontHeight()) >> 1;
  MovePen(middle_x, middle_y);
  switch (button->font_color) {
  case kRed:
    SetPenColor("red");
    break;
  case kBlack:
    SetPenColor("black");
    break;
  case kWhite:
    SetPenColor("white");
    break;
  }
  DrawTextString(button->caption);
}

// Get the length of the [left, right) characters in a string
int GetStringWidthN (char* str, int left, int right) {
  if (right > strlen(str)) {
    return -1;
  }
  char tmp = str[right];
  str[right] = '\0';
  int ret = TextStringWidth(str + left);
  str[right] = tmp;
  return ret;
}

// Draw [left, right) of the string str
void DrawTextStringN (char* str, int left, int right) {
  char tmp = str[right];
  str[right] = '\0';
  DrawTextString(str + left);
  str[right] = tmp;
}

void DrawContent(InputBox* input_box) {
  SetFont(FONT_INPUT);
  SetPenColor("black");
  SetPointSize(input_box->font_size);
  int inner_length = input_box->position.right - input_box->position.left - 10;
  int len = strlen(input_box->context);
  int left_most = 0, right_most = 0; // display string [left_most, right_most)
  if (GetStringWidthN(input_box->context, 0, input_box->cursor) <= inner_length) {
    left_most = 0;
    right_most = len;
    for (int i = input_box->cursor; i <= len; i++) {
      if (GetStringWidthN(input_box->context, left_most, i) > inner_length) {
        right_most = i - 1;
        break;
      }
    }
  } else {
    left_most = 0;
    right_most = input_box->cursor;
    for (int i = input_box->cursor; i >= 0; i--) {
      if (GetStringWidthN(input_box->context, i, right_most) > inner_length) {
        left_most = i + 1;
        break;
      }
    }
  }
  // Draw the text string
  MovePen(input_box->position.left + 5, input_box->position.bottom - 5);
  DrawTextStringN(input_box->context, left_most, right_most);
  // Draw cursor
  SetPenSize(1);
  MovePen(input_box->position.left + 5 + GetStringWidthN(input_box->context, left_most, input_box->cursor),
          input_box->position.bottom - 5);
  DrawLine(0, - GetFontHeight() + 5);
}

void DrawInputBox(InputBox* input_box) {
  SetFont(FONT_INPUT);
  SetPointSize(input_box->font_size);
  switch (input_box->status) {
    case kNormal:
      SetPenSize(2);
      SetPenColor("black");
      break;
    case kHover:
      SetPenSize(4);
      SetPenColor("red");
      break;
  }
  DrawRectangle(&input_box->position);
  DrawContent(input_box);
}

void DrawLink(Link* link) {
  SetFont(FONT_LINK);
  SetPointSize(link->font_size);
  SetPenSize(1);
  switch (link->font_color) {
  case kRed:
    SetPenColor("red");
    break;
  case kWhite:
    SetPenColor("white");
    break;
  case kBlack:
    SetPenColor("black");
    break;
  }
  switch(link->status) {
    case kHover:
      MovePen(link->position.left, link->position.bottom);
      DrawLine(TextStringWidth(link->caption), 0);
    case kNormal:
      MovePen(link->position.left, link->position.bottom);
      DrawTextString(link->caption);
      break;
  }
}

void DrawLabel(Label* label) {
  SetFont(FONT_LABEL);
  SetPointSize(label->font_size);
  switch (label->font_color)
  {
  case kRed:
    SetPenColor("red");
    break;
  case kBlack:
    SetPenColor("black");
    break;
  case kWhite:
    SetPenColor("white");
    break;
  }
  MovePen(label->position.left, label->position.bottom);
  DrawTextString(label->caption);
}

void DrawFramwork() {
  for (PTCNode p = frame->next; p != frame; p = p->next) {
    Frame* color_rect = (Frame*)p->component;
    DrawFrame(color_rect);
  }
}

/* Events handler */

extern void DisplayClear(void);

// Check whether (x, y) is in rectangle rect
int Inbox(int x, int y, Rect* rect) {
  if (rect->left >= x) return 0;
  if (rect->right <= x) return 0;
  if (rect->bottom <= y) return 0;
  if (rect->top >= y) return 0;
  return 1;
}

// Display components according to the position of the mouse
void DisplayAnimateComponents(CompList L, int x, int y) {
  Button* button = NULL;
  InputBox* input_box = NULL;
  Link* link = NULL;
  Label* label = NULL;
  Rect* rect = NULL;
  for (PTCNode p = L->next; p != L; p = p->next) {
    switch (p->type) {
    case kButton:
      button = (Button*)p->component;
      rect = &button->position;
      if (Inbox(x, y, rect)) {
        button->status = kHover;
      } else {
        button->status = kNormal;
      }
      DrawButton(button, x);
      break;
    case kInputBox:
      input_box = (InputBox*)p->component;
      rect = &input_box->position;
      if (Inbox(x, y, rect)) {
        input_box->status = kHover;
      } else {
        input_box->status = kNormal;
      }
      DrawInputBox(input_box);
      break;
    case kLink:
      link = (Link*)(p->component);
      rect = &link->position;
      if (Inbox(x, y, rect)) {
        link->status = kHover;
      } else {
        link->status = kNormal;
      }
      DrawLink(link);
      break;
    case kLabel:
      label = (Label*)(p->component);
      DrawLabel(label);
      break;
    }
    if (p == focus) {
      DrawOuter(rect);
    }
  }
}

void FlushScreen(int x, int y) {
  DisplayClear();
  DrawFramwork();
  DisplayAnimateComponents(cur_list, x, y);
  DisplayAnimateComponents(surface, x, y);
}

// Press enter on focus or click it
void PushButton() {
  if (focus->type == kButton && focus != cur_list) {
    CallbackById(((Button*)focus->component)->id);
  }
  else if (focus->type == kLink) {
    CallbackById(((Link*)focus->component)->id);
  }
}

// Surface will only contain button
void HandleClickOnSur(int x, int y, int mouse_button, int event) {
  if (surface == NULL || surface->next == surface) {
    // The surface layer is empty
    return;
  }
  Button* button = NULL;
  Rect* rect = NULL;
  switch (event) {
  case BUTTON_UP:
    if (mouse_button == LEFT_BUTTON) {
      int click_on_margin = 1;  // whether this click is on none of the components
      int id = 0;               // 0 means doing nothing
      for (PTCNode p = surface->next; p != surface; p = p->next) {
        button = (Button*)p->component;
        rect = &button->position;
        if (Inbox(x, y, rect)) {
          click_on_margin = 0;
          id = button->id;
        }
      }
      if (click_on_margin) {
        ExitSurface();
      } else {
        CallbackById(id);
      }
    }
    break;
  }
  FlushScreen(x, y);
}

void HandleClickOnComp(int x, int y, int mouse_button, int event) {
  Button* button = NULL;
  InputBox* input_box = NULL;
  Link* link = NULL;
  Label* label = NULL;
  Rect* rect = NULL;
  int click_on_margin = 1;  // whether this click is on none of the components
  switch(event) {
    case BUTTON_UP:
      if (mouse_button == LEFT_BUTTON) {
        for (PTCNode p = cur_list->next; p != cur_list; p = p->next) {
          switch (p->type) {
            case kButton:
              button = (Button*)p->component;
              rect = &button->position;
              if (Inbox(x, y, rect)) {
                focus = p;
                click_on_margin = 0;
              }
              break;
            case kInputBox:
              input_box = (InputBox*)p->component;
              rect = &input_box->position;
              if (Inbox(x, y, rect)) {
                focus = p;
                click_on_margin = 0;
              }
              break;
            case kLink:
              link = (Link*)(p->component);
              rect = &link->position;
              if (Inbox(x, y, rect)) {
                focus = p;
                click_on_margin = 0;
              }
              break;
          }
          
        }
        if (click_on_margin) {
          focus = cur_list;
        }
        PushButton();
      }
      break;
  }
  FlushScreen(x, y);
}

// Move focus to the next components
void MoveFocus() {
  focus = focus->next;
  while(focus->type == kLabel) {
    focus = focus->next;
  }
}

// Move cursor in the input box from p to p + delta, note
// that delta can only be 1/-1/INF/-INF to handle Chinese
void MoveCursor(int delta) {
  if (focus->type != kInputBox) {
    return;
  }
  InputBox* input_box = (InputBox*)focus->component;
  int len = strlen(input_box->context);
  if (input_box->cursor + delta > len) {
    input_box->cursor = len;
  } else if (input_box->cursor + delta < 0) {
    input_box->cursor = 0;
  } else {
    input_box->cursor += delta;
    if (input_box->cursor > 0 && !isascii(input_box->context[input_box->cursor - 1])) {
      switch (delta) {
        case 1:
          input_box->cursor++;
          break;
        case -1:
          input_box->cursor--;
          break;
      }
    }
  }
}

// Insert a char ch between str[position - 1] & str[position]
void InsertChar(char* str, char ch, int position) {
  int len = strlen(str);
  for (int i = len; i > position; i--) {
    str[i] = str[i - 1];
  }
  str[position] = ch;
}

void DeleteChar(char* str, int position) {
  int len = strlen(str);
  if (position >= len || position < 0) {
    return;
  }
  for (int i = position; i < len; i++) {
    str[i] = str[i + 1];
  }
}

void ChangeInputBox(int key) {
  if (focus->type != kInputBox) {
    return;
  }
  if (!iscntrl(key)) {
    InputBox* input_box = (InputBox*)focus->component;
    InsertChar(input_box->context, key, input_box->cursor);
    MoveCursor(1);
  }
}

void DeleteInputBox() {
  if (focus->type != kInputBox) {
    return;
  }
  InputBox* input_box = (InputBox*)focus->component;
  DeleteChar(input_box->context, input_box->cursor);
}

void BackSpaceInputBox() {
  if (focus->type != kInputBox) {
    return;
  }
  InputBox* input_box = (InputBox*)focus->component;
  DeleteChar(input_box->context, input_box->cursor - 1);
  MoveCursor(-1);
}

// Handle the mouse movement on components
void MouseMoveEventHandler(int x, int y, int mouse_button, int event) {
  HandleClickOnSur(x, y, mouse_button, event);
  HandleClickOnComp(x, y, mouse_button, event);
  FlushScreen(x, y);
}

void KeyboardEventHandler(int key, int event) {
  if(event == KEY_UP) {
    switch (key) {
      case VK_TAB:
        MoveFocus();
        break;
      case VK_LEFT:
        MoveCursor(-1);
        break;
      case VK_RIGHT:
        MoveCursor(1);
        break;
      case VK_UP:
        MoveCursor(-1000);
        break;
      case VK_DOWN:
        MoveCursor(1000);
        break;
      case VK_DELETE:
        DeleteInputBox();
        break;
      case VK_BACK:
        BackSpaceInputBox();
        break;
      case VK_RETURN:
        PushButton();
    }
  }
  FlushScreen(GetMouseX(), GetMouseY());
}

void CharEventHandler(int key) {
  ChangeInputBox(key);
  FlushScreen(GetMouseX(), GetMouseY());
}

void ClearComponents() {
  if (cur_list != NULL) {
    FreeList(cur_list);
  }
}

void ClearSurface() {
  if (surface != NULL) {
    FreeList(surface);
  }
}

void ClearFrame() {
  if (frame != NULL) {
    FreeList(frame);
  }
}

// Initialize the components
void InitComponents() {
  ClearComponents();
  cur_list = NewCompList();
  focus = cur_list;
}

void InitFrame() {
  ClearFrame();
  frame = NewCompList();
}

void InitSurface() {
  ClearSurface();
  surface = NewCompList();
}

// Initialization of this set of GUI components
void InitializeUI() {
  static int registered = 0;
  InitComponents();
  InitFrame();
  InitSurface();
  if (registered == 0) {
    registerMouseEvent(MouseMoveEventHandler);
    registerKeyboardEvent(KeyboardEventHandler);
    registerCharEvent(CharEventHandler);
    registered = 1;
  }
}