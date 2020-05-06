#include "exception.h"
#include "genlib.h"
#include "graphics.h"

void Draw(LibImage img, const double cx, const double cy) {
  const double ratio = 1.0 * img.height / img.width;
  DrawImage(&img, cx - 1, cy - ratio, 2, 2 * ratio);
}

void Main() {
  InitGraphics();
  InitConsole();
  LibImage img;
  try {
    char path[200];
    SelectFile("JPG image (*.jpg|*.jpeg|*.jpe)\0*.jpg;*.jpeg;*.jpe\0", "jpg",
               FALSE, path, 200 - 1);
    puts(path);
    loadImage(path, &img);
    except(ErrorException) puts("oops");
  } endtry;
  Draw(img, GetWindowWidth() / 2, GetWindowHeight() / 2);
}
