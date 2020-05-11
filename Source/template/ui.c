#include "gui.h"
#include "ui.h"
#include "graphics.h"
#include "extgraph.h"
#include <wingdi.h>
#include <Windows.h>

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

/* Draw Components */
Button* CreateButton(Rect rect, char* caption) {
  Button* ret = malloc(sizeof(Button));
  ret->position = rect;
  strcpy(ret->caption, caption);
  return ret;
}

InputBox* CreateInputBox(Rect rect, char* context) {
  InputBox* ret = malloc(sizeof(InputBox));
  ret->position = rect;
  ret->cursor = 0;
  strcpy(ret->context, context);
  return ret;
}


/* Singly linked circular list for components */

static PTCNode Focus;
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
  new_list->type = 0;
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
  Focus = cur_list;
}

/* End of linked list */

/*
 * Handle events about the components
 * MoveFocus(): Move to the next components
 * Inbox(x, y, button): check whether (x, y) is inside
 */

// Move focus to the next components
void MoveFocus() {
  Focus = Focus->next;
}

int Inbox(int x, int y, Rect* rect) {
  if (rect->left >= x) return 0;
  if (rect->right <= x) return 0;
  if (rect->bottom <= y) return 0;
  if (rect->top >= y) return 0;
  return 1;
}

// Handle the mouse movement on components
void MouseMoveEventHandler(int x, int y, int mouse_button, int event) {
  Rect* rect = NULL;
  Button* button = NULL;
  InputBox* input_box = NULL;
  for (PTCNode p = cur_list->next; p != cur_list; p = p->next) {
    switch (p->type) {
      case kButton:
        button = (Button*)p->component;
        rect = &(button->position);
        if (Inbox(x, y, rect)) {
          DrawButton(button, x, 1);
        } else {
          DrawButton(button, x, 0);
        }
        break;
      case kInput:
        input_box = (InputBox*)p->component;
        rect = &(input_box->position);
        if (Inbox(x, y, rect)) {
          DrawInputBox(input_box, 1);
        } else {
          DrawInputBox(input_box, 0);
        }
        break;
    }
  }
}

/* End of input events*/

void DrawButton(Button* button, int mouse_x, int highlight) {
  // Some color for material design
  Color light_blue = ColorConvert("2196F3", 1);
  Color white = ColorConvert("E3F2FD", 1);
  ColorPoint upper_left = (ColorPoint){button->position.left, button->position.top, light_blue};
  ColorPoint lower_right = (ColorPoint){button->position.right, button->position.bottom, light_blue};
  if (highlight) {
    ColorPoint upper_middle = (ColorPoint){mouse_x, button->position.top, white};
    ColorPoint lower_middle = (ColorPoint){mouse_x, button->position.bottom, white};
    DrawShadedRectangle(&lower_middle, &upper_left);
    DrawShadedRectangle(&lower_right, &upper_middle);
  } else {
    DrawShadedRectangle(&lower_right, &upper_left); 
  }
  int middle_x = (button->position.left + button->position.right
                  - TextStringWidth(button->caption)) >> 1;
  int middle_y = (button->position.top + button->position.bottom) >> 1;
  MovePen(middle_x, middle_y);
  DrawTextString(button->caption);
}

void DrawInputBox(InputBox* input_box, int highlight) {
  MovePen(input_box->position.left, input_box->position.top);
  if (highlight) {
    SetPenSize(2);
    SetPenColor("red");
  } else {
    SetPenSize(2);
    SetPenColor("black");
  }
  DrawLine(input_box->position.right - input_box->position.left, 0);
  DrawLine(0, input_box->position.bottom - input_box->position.top);
  DrawLine(input_box->position.left - input_box->position.right, 0);
  DrawLine(0, input_box->position.top - input_box->position.bottom);
}

void DrawComponents() {
  for (PTCNode p = cur_list->next; p != cur_list; p = p->next) {
    switch (p->type) {
      case kButton:
        DrawButton((Button*)p->component, -1, 0);
        break;
      case kInput:
        DrawInputBox((InputBox*)p->component, 0);
        break;
    }
  }
}
