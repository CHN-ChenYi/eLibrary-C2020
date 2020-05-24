#pragma once
#include "graphics.h"


typedef enum {
  kNormal,  // the normal status
  kHover,   // when the mouse is hovering on it
} ComponentStatus;

typedef enum { kRed, kBlack, kWhite } FontColor;

/*
 * 组件
 * Button   : 按钮
 * InputBox : 输入框
 * Link     : 链接
 * Label    : 标签
 * Frame    : 框架
 * Image    : 图片
 */

/*
 * 重要声明：
 * libgraphics的设置字体大小的函数和设置字体的函数会导致内存泄漏
 * （次数较少时看不出来，但是测试的时候足够多次操作能明显看出）
 * 因此，下面所有的元件都删除了字体大小和字体样式的字段
 */

/* Button */
typedef struct Button {
  int id;  // for callback
  ComponentStatus status;
  Rect position;         // position
  char caption[10];      // label on the button
  Color bg_color;        // background color
  FontColor font_color;  // font color
} Button;

/* Input box */
typedef struct InputBox {
  int id;  // for callback
  ComponentStatus status;
  Rect position;      // position
  char context[500];  // context (already input)
  int cursor;         // position of the cursor
  int is_terminal;
} InputBox;

/* Link */
typedef struct Link {
  int id;  // for callback
  ComponentStatus status;
  Rect position;  // only the left and the bottom matters
  FontColor font_color;
  char caption[400];
} Link;

/* Label */
typedef struct Label {
  int id;
  Rect position;  // only the left and the buttom matters
  FontColor font_color;
  char caption[400];
} Label;

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

/*
 * 组件共同属性说明：
 * rect       : 一个矩形，其大小由左右上下确定
 * font_color : 一个enum类型，只有三种颜色
 *              kRed    : 红色
 *              kBlack  : 黑色
 *              kWhite  : 白色
 * id         : 用来标记回调函数，当某元件被点击的时候，调用其id对应的回调函数
 */

/*
 * CreateButton(rect, caption, bg_color, alpha, font_color, id);
 * 创建一个按钮，返回新创建的按钮元件的地址
 * caption  : 按钮上显示的文字
 * bg_color : 按钮的背景色，采用CSS样式的长度为6的串
 * alpha    : 按钮背景色的透明度
 */
Button* CreateButton(Rect rect, char* caption, char* bg_color, double alpha,
                     FontColor font_color, int id);

/*
 * CreateInputBox(rect, str, id, is_terminal);
 * 创建一个输入框，并返回新创建的输入框元件的地址
 * rect        : 此处只需要输入左右边界和下边界位置即可，上边界会根据字体自动调整
 * str         : 初始字符串
 * is_terminal : 底部的状态信息栏也是使用InputBox，但是不响应删除操作，颜色相反
 *               因此需和一般的输入框区分，此处1表示是底部状态栏，0表示不是
 */
InputBox* CreateInputBox(Rect rect, char* str, int id, int is_terminal);

/*
 * CreateLink(rect, caption, font_color, id);
 * 创建一个链接，并返回新创建的链接的地址
 */
Link* CreateLink(Rect rect, char* caption, FontColor font_color, int id);
Label* CreateLabel(Rect rect, char* caption, FontColor font_color, int id);
Frame* CreateFrame(Rect rect, char* color, double alpha);
Image* CreateImage(Rect rect, LibImage ui_image, int id);

/* Singly linked circular list for components */
typedef struct ComponentListNode* PTCNode;
typedef enum { kButton, kInputBox, kLink, kLabel, kImage } TypeOfComp;
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
void FlushScreen(int x, int y);

// Initialization
void InitializeUI();