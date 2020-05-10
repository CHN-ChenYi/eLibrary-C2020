#include "gui.h"
#include "ui.h"
#include "graphics.h"
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

/* Handle the difference between scale & pixels */

int ScaleToPixelX(double x) {
  return 
}

/* Singly linked circular list for components */
typedef struct ComponentListNode* PTCNode;
typedef enum {kButton, kInput} TypeOfComp;
struct ComponentListNode {
  TypeOfComp type;  // The type of the component
  void* component;  // pointer to the component
  PTCNode next;
};

typedef PTCNode CompList;
static PTCNode Focus;

// Add a new component into the list
void InsertComp(void* component, TypeOfComp type, CompList L) {
  PTCNode new_node = (PTCNode)malloc(sizeof(struct ComponentListNode));
  new_node->component = component;
  new_node->type = type;
  new_node->next = L->next;
  L->next = new_node;
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
void FreeCompList(CompList L) {
  for (PTCNode p = L, next = L->next; p != NULL; p = next, next = p->next) {
    free(p);
  }
}

CompList cur_list;  // The list of all the components on the current page.

/* End of linked list */

/*
 * Handle events about the components
 * MoveFocus(): Move to the next components
 * Inbox(x, y, button): check whether (x, y) is inside
 */

// Move focus to the next components
void MoveFocus() { Focus = Focus->next; }

int Inbox(int x, int y, Rect* rect) {
  if (rect->left >= x) return 0;
  if (rect->right <= x) return 0;
  if (rect->bottom <= y) return 0;
  if (rect->top >= y) return 0;
  return 1;
}

// Handle the mouse movement on components
void MouseMoveEventHandler(int x, int y, int mouse_button, int event) {
  for (PTCNode p = cur_list->next; p != cur_list; p = p->next) {
    Rect* rect;
    Button* button;
    InputBox* input_box;
    switch (p->type) {
      case kButton:
        button = (Button*)p->component;
        rect = &button->position;
        if (Inbox(x, y, rect)) {
          DrawButton(button, x, 1);
        }
        break;
      case kInput:
        input_box = (InputBox*)p->component;
        rect = &input_box->position;
        if (Inbox(x, y, rect)) {
          DrawInputBox(input_box, 1);
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
}

void DrawInputBox(InputBox* input_box, int highlight) {
  
}