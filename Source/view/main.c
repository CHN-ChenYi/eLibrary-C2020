#include "exception.h"
#include "graphics.h"

void Main() {
  InitGraphics();
  InitConsole();
  const double cx = GetWindowWidth() / 2;
  const double cy = GetWindowHeight() / 2;
  LibImage img;
  try {
    loadImage("./Resource/test.jpg", &img);
    except(ErrorException) puts("oops");
  } endtry
  DrawImage(&img, cx - 1, cy - 1.5, 2, 3);
}
