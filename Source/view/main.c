#include "exception.h"
#include "genlib.h"
#include "graphics.h"
#include "extgraph.h"
#include "page.h"
#include "gui.h"
#include "ui.h"


void Main() {
  //InitGUI();
  InitGraphics();
  InitConsole();
  InitializeUI();
  InputBox *input_box = CreateInputBox(
    (Rect){100, 200, 100, 200}, "", 0
  );
  InsertComp(input_box, kInputBox);
  
}