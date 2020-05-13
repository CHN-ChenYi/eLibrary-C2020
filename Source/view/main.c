#include "exception.h"
#include "genlib.h"
#include "graphics.h"
#include "extgraph.h"
#include "ui.h"

void Main() {
  InitGraphics();
  InitConsole();

  Button* button = CreateButton((Rect){100, 300, 100, 200}, "你好", 30, 1);
  InputBox* input_box = CreateInputBox((Rect){100, 300, 350, 450}, 30, 2);
  Link* link = CreateLink((Rect){100, 300, 500, 600}, "点这里", 30, 3);
  Label* label = CreateLabel((Rect){100, 300, 650, 750}, "fuck", 30, 3);

  strcpy(input_box->context, "你好");

  InitComponents();
  InsertComp(button, kButton);
  InsertComp(input_box, kInputBox);
  InsertComp(link, kLink);
  InsertComp(label, kLabel);
  DrawComponents();
  registerMouseEvent(MouseMoveEventHandler);
  registerKeyboardEvent(KeyboardEventHandler);
  registerCharEvent(CharEventHandler);
}
