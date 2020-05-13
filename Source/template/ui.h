#pragma once

// Color with alpha value consistent with wingdi
typedef struct Color {
  int R, G, B;
  int Alpha;
} Color;

/* Point with color and position specified */
typedef struct ColorPoint {
  int x, y;
  Color color;
} ColorPoint;

/* Rectangle (for position specifying) */
typedef struct Rect {
  int left, right, top, bottom;
} Rect;

/*
 * Components:
 * Button   : a button for callback
 * InputBox : input box for message collection
 * Link     : a link to other page
 * Label    : a label to show text
 */

typedef enum {
  kNormal,  // the normal status
  kHover,   // when the mouse is hovering on it
} ComponentStatus;

/* Button */
typedef struct Button {
  int id;              // for callback
  ComponentStatus status;
  Rect position;       // position
  int font_size;       // size of font
  char caption[10];    // label on the button
} Button;

/* Input box */
typedef struct InputBox {
  int id;             // for callback
  ComponentStatus status;
  Rect position;      // position
  int font_size;      // size of font
  char context[500];  // context (already input)
  int cursor;         // position of the cursor
} InputBox;

/* Link */
typedef struct Link {
  int id;             // for callback
  ComponentStatus status;
  Rect position;      // only the left and the bottom matters
  int font_size;      // size of font
  char caption[20];
}Link;

/* Label */
typedef struct Label {
  int id;
  Rect position;      // only the left and the buttom matters
  int font_size;      // size of font
  char caption[500];
}Label;

/* Rectangle with color (for framework) */
typedef struct Frame {
  Rect position;
  Color color;
} ColorRect;

/* Create UI components */
Button* CreateButton(Rect rect, char* caption, int font_size, int id);
InputBox* CreateInputBox(Rect rect, int font_size, int id);
Link* CreateLink(Rect rect, char* caption, int font_size, int id);
Label* CreateLabel(Rect rect, char* caption, int font_size, int id);
ColorRect* CreateFrame(Rect rect, char* color, double alpha);

/* Draw UI components */
void DrawButton(Button* button, int mouse_x);
void DrawInputBox(InputBox* input_box);
void DrawLink(Link* link);
void DrawLabel(Label* label);
void DrawFrame(ColorRect* rect);

/* Event handlers */
void MouseMoveEventHandler(int x, int y, int mouse_button, int event);
void KeyboardEventHandler(int key, int event);
void CharEventHandler(int key);

/* Singly linked circular list for components */
typedef struct ComponentListNode* PTCNode;
typedef enum {
  kButton,
  kInputBox,
  kLink,
  kLabel
} TypeOfComp;
struct ComponentListNode {
  TypeOfComp type;  // The type of the component
  void* component;  // pointer to the component
  PTCNode next;
};

typedef PTCNode CompList;

void InsertComp(void* component, TypeOfComp type);
void InsertFrame(void* component);
void FreeCompList();
void DrawComponents();
void DrawFramwork(); 

// Initialization
void InitializeUI();