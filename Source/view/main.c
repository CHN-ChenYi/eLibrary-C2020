#include "exception.h"
#include "genlib.h"
#include "graphics.h"
#include "extgraph.h"
#include "page.h"
#include "gui.h"


void Main() {
  InitGUI();
  char *terminal = malloc(sizeof(char) * 100);
  memset(terminal, 0, sizeof(terminal));
  strcpy(terminal, "我完蛋了");
  DrawUI(kUserModify, NULL, NULL, terminal);
}