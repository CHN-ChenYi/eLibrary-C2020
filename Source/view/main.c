#include "exception.h"
#include "genlib.h"
#include "graphics.h"
#include "extgraph.h"
#include "ui.h"

void Main() {
  InitGraphics();
  InitConsole();

  InitializeUI();
  Button* button = CreateButton((Rect){100, 500, 100, 200}, "你好", 30, "64B5F6", 1, kWhite, 1);
  InsertComp(button, kButton);
}
