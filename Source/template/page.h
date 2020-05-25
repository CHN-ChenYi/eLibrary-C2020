#pragma once
#include "gui.h"

void InitPage();            // 新建一个界面
void CallbackById(int id);  // 处理id为id的回调
void ExitSurface();         // 清除表层
void InitGUI();             // 初始化GUI模块
void HandleCtrl(int key);   // 处理键盘快捷键