#include "exception.h"
#include "genlib.h"
#include "graphics.h"
#include "extgraph.h"
#include "page.h"
#include "gui.h"


void Main() {
  InitGUI();
  char *terminal = malloc(sizeof(char) * 100);
  strcpy(terminal, "我完蛋了");
  DrawUI(kBookSearch, NULL, NULL, terminal);
}