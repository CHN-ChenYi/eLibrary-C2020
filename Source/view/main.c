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
    char path[MAX_PATH + 1];
    SelectFile("JPG image (*.jpg|*.jpeg|*.jpe)\0*.jpg;*.jpeg;*.jpe\0", "jpg",
               FALSE, path, MAX_PATH);
    puts(path);
    loadImage(path, &img);
    except(ErrorException) puts("oops");
  } endtry;
  Draw(img, GetWindowWidth() / 2, GetWindowHeight() / 2);

  char path[MAX_PATH + 1];
  try {
    SelectFolder("请选择保存图书库的文件夹", path);
    except(ErrorException) puts("oops");
  } endtry;
  puts(path);
}
