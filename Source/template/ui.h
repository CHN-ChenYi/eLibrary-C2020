#pragma once

// 带透明度的颜色，均为十六进制
// 范围均为：[0, 65536)
typedef struct Color {
  int R, G, B;
  int Alpha;
} Color;

typedef struct ColorPoint {
  int x, y;
  Color color;
} ColorPoint;


typedef struct Rect {
  int left, right, top, bottom;
} Rect;

typedef struct Button {
  Rect position;       // 位置
  char caption[10];    // 字幕
} Button;

typedef struct InputBox {
  Rect position;
  char context[500];
} InputBox;

/* Draw UI components */
void DrawButton(Button* button, int mouse_x, int highlight);
void DrawInputBox(InputBox* input_box, int highlight);

/* Event handlers */
void MouseMoveEventHandler(int x, int y, int mouse_button, int event);

/* Others */
Color ColorConvert(char* color, double alpha);

/* Singly linked circular list for components */
typedef struct ComponentListNode* PTCNode;
typedef enum { kButton, kInput } TypeOfComp;
struct ComponentListNode {
  TypeOfComp type;  // The type of the component
  void* component;  // pointer to the component
  PTCNode next;
};

typedef PTCNode CompList;

void InitComponents();
void InsertComp(void* component, TypeOfComp type);
void FreeCompList();
void DrawComponents();