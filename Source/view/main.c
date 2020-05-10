#include "exception.h"
#include "genlib.h"
#include "graphics.h"
#include "ui.h"

void Main() {
  InitGraphics();
  InitConsole();

  Button button = (Button){(Rect){100, 500, 100, 500}, "Fuck you!"};
  DrawButton(&button, 300.0, 1);

}
