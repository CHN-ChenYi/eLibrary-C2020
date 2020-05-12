#include "gui.h"
#include "ui.h"
#include "graphics.h"
#include "extgraph.h"
#include <wingdi.h>
#include <Windows.h>

#define FONT "Consolas"

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

static PTCNode focus;      // The current focus
static CompList cur_list;  // The list of all the components on the current page.

// Add a new component into the list
void InsertComp(void* component, TypeOfComp type) {
  PTCNode new_node = (PTCNode)malloc(sizeof(struct ComponentListNode));
  new_node->component = component;
  new_node->type = type;
  new_node->next = cur_list->next;
  cur_list->next = new_node;
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
void FreeCompList() {
  for (PTCNode p = cur_list, next = cur_list->next; p != NULL; p = next, next = p->next) {
    free(p->component);
    free(p);
  }
}

void InitComponents() {
  if (cur_list != NULL) {
    FreeCompList(cur_list);
  }
  cur_list = NewCompList();
  focus = cur_list;
}

/* End of linked list */

/* Create components */
Button* CreateButton(Rect rect, char* caption, int font_size, int id) {
  Button* ret = malloc(sizeof(Button));
  ret->id = id;
  ret->position = rect;
  ret->status = kNormal;
  ret->font_size = font_size;
  strcpy(ret->caption, caption);
  return ret;
}

InputBox* CreateInputBox(Rect rect, int font_size, int id) {
  InputBox* ret = malloc(sizeof(InputBox));
  ret->id = id;
  ret->position = rect;
  ret->cursor = 0;
  ret->status = kNormal;
  ret->font_size = font_size;
  memset(ret->context, 0, sizeof(ret->context));
  return ret;
}

Link* CreateLink(Rect rect, char* caption, int font_size, int id) {
  Link* ret = malloc(sizeof(Link));
  ret->id = id;
  ret->position = rect;
  SetFont(FONT);
  SetPointSize(font_size);
  ret->position.right = ret->position.left + TextStringWidth(caption);
  ret->position.top = ret->position.bottom - GetPointSize();
  ret->status = kNormal;
  ret->font_size = font_size;
  strcpy(ret->caption, caption);
  return ret;
}

Label* CreateLabel(Rect rect, char* caption, int font_size, int id) {
  Label* ret = malloc(sizeof(Label));
  ret->id = id;
  ret->position = rect;
  SetFont(FONT);
  SetPointSize(font_size);
  ret->position.right = ret->position.left + TextStringWidth(caption);
  ret->position.top = ret->position.bottom - GetPointSize();
  ret->font_size = font_size;
  strcpy(ret->caption, caption);
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

/* Draw a rectangle to wrap it*/
void DrawOuter(Rect* rect) {
  SetPenColor("red");
  SetPenSize(1);
  MovePen(rect->left - 5, rect->top - 5);
  DrawLine(rect->right - rect->left + 10, 0);
  DrawLine(0, rect->bottom - rect->top + 10);
  DrawLine(rect->left - rect->right - 10, 0);
  DrawLine(0, rect->top - rect->bottom - 10);
}

void DrawButton(Button* button, int mouse_x) {
  SetFont(FONT);
  SetPointSize(button->font_size);
  SetPenSize(1);
  SetPenColor("black");
  // Some color for material design
  Color light_blue = ColorConvert("2196F3", 1);
  Color white = ColorConvert("E3F2FD", 1);
  ColorPoint upper_left = (ColorPoint){button->position.left, button->position.top, light_blue};
  ColorPoint lower_right = (ColorPoint){button->position.right, button->position.bottom, light_blue};
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
  DrawTextString(button->caption);
}

void DrawInputBox(InputBox* input_box) {
  SetPointSize(input_box->font_size);
  SetPenSize(1);
  switch (input_box->status) {
    case kNormal:
      SetPenColor("black");
      break;
    case kHover:
      SetPenColor("red");
      break;
  }
  DrawRectangle(&input_box->position);
}

void DrawLink(Link* link) {
  SetFont(FONT);
  SetPointSize(link->font_size);
  SetPenSize(1);
  switch(link->status) {
    case kHover:
      SetPenColor("blue");
      MovePen(link->position.left, link->position.bottom);
      DrawLine(TextStringWidth(link->caption), 0);
    case kNormal:
      MovePen(link->position.left, link->position.bottom);
      DrawTextString(link->caption);
      break;
  }
}

void DrawLabel(Label* label) {
  SetFont(FONT);
  SetPointSize(label->font_size);
  SetPenSize(1);
  SetPenColor("black");
  MovePen(label->position.left, label->position.bottom);
  SetPenSize(label->font_size);
  DrawTextString(label->caption);
}

void DrawComponents() {
  for (PTCNode p = cur_list->next; p != cur_list; p = p->next) {
    switch (p->type) {
      case kButton:
        DrawButton((Button*)p->component, -1);
        break;
      case kInputBox:
        DrawInputBox((InputBox*)p->component);
        break;
      case kLabel:
        DrawLabel((Label*)p->component);
        break;
      case kLink:
        DrawLink((Link*)p->component);
        break;
    }
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
void DisplayAnimateComponents(int x, int y) {
  DisplayClear();
  Button* button = NULL;
  InputBox* input_box = NULL;
  Link* link = NULL;
  Label* label = NULL;
  Rect* rect = NULL;
  for (PTCNode p = cur_list->next; p != cur_list; p = p->next) {
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

void HandleClick(int x, int y, int mouse_button, int event) {
  Button* button = NULL;
  InputBox* input_box = NULL;
  Link* link = NULL;
  Label* label = NULL;
  Rect* rect = NULL;
  switch(event) {
    case BUTTON_DOWN:
      if (mouse_button == LEFT_BUTTON) {
        for (PTCNode p = cur_list->next; p != cur_list; p = p->next) {
          switch (p->type) {
            case kButton:
              button = (Button*)p->component;
              rect = &button->position;
              if (Inbox(x, y, rect)) {
                focus = p;
                DisplayAnimateComponents(x, y);
              }
              break;
            case kInputBox:
              input_box = (InputBox*)p->component;
              rect = &input_box->position;
              if (Inbox(x, y, rect)) {
                focus = p;
                DisplayAnimateComponents(x, y);
              }
              break;
            case kLink:
              link = (Link*)(p->component);
              rect = &link->position;
              if (Inbox(x, y, rect)) {
                focus = p;
                DisplayAnimateComponents(x, y);
              }
              break;
          }
        }
      }
      break;
  }
}

// Move focus to the next components
void MoveFocus() {
  focus = focus->next;
  while(focus->type == kLabel) {
    focus = focus->next;
  }
}

// Handle the mouse movement on components
void MouseMoveEventHandler(int x, int y, int mouse_button, int event) {
  DisplayAnimateComponents(x, y);
  HandleClick(x, y, mouse_button, event);
}

void KeyboardEventHandler(int key, int event) {
  if(event == KEY_UP) {
    switch (key) {
      case VK_TAB:
        MoveFocus();
        break;
    }
  }
  DisplayAnimateComponents(GetMouseX(), GetMouseY());
}
