#pragma once
#include "graphics.h"


typedef enum {
  kNormal,  // the normal status
  kHover,   // when the mouse is hovering on it
} ComponentStatus;

typedef enum {
  kRed,
  kBlack,
  kWhite
} FontColor;

/*
 * Components:
 * Button   : a button for callback
 * InputBox : input box for message collection
 * Link     : a link to other page
 * Label    : a label to show text
 */

/* Button */
typedef struct Button {
  int id;                   // for callback
  ComponentStatus status;
  Rect position;            // position
  char caption[10];         // label on the button
  Color bg_color;           // background color
  FontColor font_color;     // font color
} Button;

/* Input box */
typedef struct InputBox {
  int id;             // for callback
  ComponentStatus status;
  Rect position;      // position
  char context[500];  // context (already input)
  int cursor;         // position of the cursor
} InputBox;

/* Link */
typedef struct Link {
  int id;             // for callback
  ComponentStatus status;
  Rect position;      // only the left and the bottom matters
  FontColor font_color;
  char caption[400];
}Link;

/* Label */
typedef struct Label {
  int id;
  Rect position;      // only the left and the buttom matters
  FontColor font_color;
  char caption[400];
}Label;

/* Rectangle with color (for framework) */
typedef struct Frame {
  Rect position;
  Color color;
} Frame;

typedef struct Image {
  Rect position;
  LibImage ui_image;  
  int id;
} Image;

/* Create UI components */
Button* CreateButton(Rect rect, char* caption, char* bg_color, double alpha, FontColor font_color, int id);
InputBox* CreateInputBox(Rect rect, char* str, int id);
Link* CreateLink(Rect rect, char* caption, FontColor font_color, int id);
Label* CreateLabel(Rect rect, char* caption, FontColor font_color, int id);
Frame* CreateFrame(Rect rect, char* color, double alpha);
Image* CreateImage(Rect rect, LibImage ui_image, int id);

/* Singly linked circular list for components */
typedef struct ComponentListNode* PTCNode;
typedef enum {
  kButton,
  kInputBox,
  kLink,
  kLabel,
  kImage
} TypeOfComp;
struct ComponentListNode {
  TypeOfComp type;  // The type of the component
  void* component;  // pointer to the component
  PTCNode next;
};

typedef PTCNode CompList;

void InsertComp(void* component, TypeOfComp type);
void InsertFrame(void* component);
void InsertSurface(void* component, TypeOfComp type);
void InitComponents();
void InitFrame();
void InitSurface();
void FlushScreen(int x, int y, int force);

// Initialization
void InitializeUI();