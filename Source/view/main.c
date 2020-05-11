#include "exception.h"
#include "genlib.h"
#include "graphics.h"
#include "ui.h"

Button *CreateButton(Rect rect, char *caption) {
  Button *ret = malloc(sizeof(Button));
  ret->position = rect;
  strcpy(ret->caption, caption);
  return ret;
}

InputBox *CreateInputBox(Rect rect, char *context) {
  InputBox *ret = malloc(sizeof(InputBox));
  ret->position = rect;
  strcpy(ret->context, context);
  return ret;
}

void Main() {
  InitGraphics();
  InitConsole();

  Button *button1 = CreateButton((Rect){100, 500, 100, 200}, "Fuck you!");
  Button *button2 = CreateButton((Rect) { 100, 500, 300, 400 }, "Fuck you!");
  Button *button3 = CreateButton((Rect){100, 500, 500, 600}, "Fuck you!");

  InputBox *input_box = CreateInputBox((Rect){100, 500, 100, 500}, "Fuck you!");
  InitComponents();
  InsertComp(button1, kButton);
  InsertComp(button2, kButton);
  InsertComp(button3, kButton);
  InsertComp(input_box, kInput);
  DrawComponents();
  registerMouseEvent(MouseMoveEventHandler);
  //MovePen(100, 100);
  //DrawLine(100, 100);

}
