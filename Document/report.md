# 1 大程序简介

## 1.1 选题背景及意义

## 1.2 目标要求

## 1.3 术语说明

# 2 功能需求分析

# 3 程序开发设计

## 3.1 总体架构设计

## 3.2 功能模块设计

## 3.3 数据结构设计

## 3.4 源代码文件组织设计

## 3.5 函数设计描述

### model

### template

##### CreateButton

* 函数原型

  ```c
  Button* CreateButton(Rect rect, char* caption, char* bg_color, double alpha,
                       FontColor font_color, int id)
  ```

* 功能描述：创建一个按钮，返回新创建的按钮元件的地址

* 参数描述：

  * rect    : 一个矩形，其大小由左右上下确定

  *  font_color : 一个enum类型，只有三种颜色

    * kRed  : 红色

    * kBlack : 黑色

    * kWhite : 白色

  * id     : 用来标记回调函数，当某元件被点击的时候，调用其id对应的回调函数

  * caption : 按钮上显示的文字

  * bg_color : 按钮的背景色，采用CSS样式的长度为6的串

  * alpha  : 按钮背景色的透明度

* 返回值描述：新创建的按钮元件的地址

* 重要局部变量定义：```Button* ret```

* 重要局部变量用途描述：暂存新生成的按钮元件

* 函数算法描述：无

##### CreateInputBox

* 函数原型

  ```c
  InputBox* CreateInputBox(Rect rect, char* str, int id, int is_terminal);
  ```

* 功能描述：创建一个输入框，并返回新创建的输入框元件的地址

* 参数描述：

  * rect: 同上 此处只需要输入左右边界和下边界位置即可，上边界会根据字体自动调整
  *  str: 初始字符串
  * id: 同上
  *  is_terminal : 底部的状态信息栏也是使用InputBox，但是不响应删除操作，颜色相反，因此需和一般的输入框区分，此处1表示是底部状态栏，0表示不是

* 返回值描述：新创建的输入框元件的地址

* 重要局部变量定义：`InputBox* ret`

* 重要局部变量用途描述：暂存新生成的输入框元件

* 函数算法描述：无

##### CreateLink

* 函数原型

  ```c
  Link* CreateLink(Rect rect, char* caption, FontColor font_color, int id);
  ```

* 功能描述：创建一个链接，并返回新创建的链接的地址

* 参数描述：

  * rect   : 此处只需要输入左下边界即可，上右边界会根据字串的长宽自动计算
  * caption : 链接的文字
  * 其余同上

* 返回值描述：新创建的链接的地址

* 重要局部变量定义：`Link* ret`

* 重要局部变量用途描述：暂存新生成的链接

* 函数算法描述：无

##### CreateLabel

- 函数原型

  ```c
  Label* CreateLabel(Rect rect, char* caption, FontColor font_color, int id);
  ```

- 功能描述：创建一个标签，并返回新创建的标签元件的地址

- 参数描述：

  - rect   : 此处只需要输入左下边界即可，上右边界会根据字串的长宽自动计算
  - caption : 标签的文字
  - 其余同上

- 返回值描述：新创建的标签元件的地址

- 重要局部变量定义：`Label* ret`

- 重要局部变量用途描述：暂存新生成的标签

- 函数算法描述：无

##### CreateFrame

- 函数原型

  ```c
  Frame* CreateFrame(Rect rect, char* color, double alpha);
  ```

- 功能描述：创建一个框架并返回指向新框架的指针

- 参数描述：

  - color : 一个CSS风格的6位字符串，表示框架的颜色
  - alpha : 框架透明度，0到1
  - 其余同上

- 返回值描述：指向新框架的指针

- 重要局部变量定义：`Frame* ret`

- 重要局部变量用途描述：暂存新生成的框架

- 函数算法描述：无

##### CreateImage

* 函数原型

  ```c
  CreateImage(Rect rect, LibImage ui_image, int id);
  ```

* 功能描述：创建一个图片元件，并返回一个指向新创建的图片元素的指针

* 参数描述：

  * rect   : 此处规定的是图片允许被占用的最大区域，当图片的长宽比恰好等于此区域的长宽比的时候，图片会刚好占到这片区域；否则将会同比缩放图片，使长宽中有一维贴合边界
  * ui_image : 图片本身

* 返回值描述：指向新创建的图片元素的指针

* 重要局部变量定义：`Image* ret`

* 重要局部变量用途描述：暂存新生成的图片元素

* 函数算法描述：无

##### InsertComp

- 函数原型

  ```c
  void InsertComp(void* component, TypeOfComp type);
  ```

- 功能描述：向当前画面插入分类为type的元素component

- 参数描述：

- 返回值描述

- 重要局部变量定义：

- 重要局部变量用途描述

- 函数算法描述

### view

#### main.c

##### Main

* 函数原型
  ```c
  void Main();
  ```

* 功能描述
* 参数描述
* 返回值描述
* 重要局部变量定义
* 重要局部变量用途描述
* 函数算法描述

# 4 部署运行和使用说明

## 4.1 编译安装

## 4.2 运行测试

## 4.3 使用操作

# 5 团队合作

## 5.1 任务分工

（略）

## 5.2 开发计划

| 日期        | 工作                 |
| ----------- | -------------------- |
| 4.21 - 4.27 | 大程开始，敲定选题   |
| 4.28 - 5.3  | 分工，完成头文件编写 |
| 5.4 - 5.12  | 同步开发             |
| 5.12 - 5.18 | 整合、调试、编写文档 |

## 5.3 编码规范

文件统一采用UTF-8编码，CRLF换行

代码风格采用谷歌的[C++ 风格指南](https://zh-google-styleguide.readthedocs.io/en/latest/google-cpp-styleguide/contents/)（删去其中C++特性的部分）

## 5.4 合作总结

## 5.5 收获感言

# 6 参考文献资料

1. [SHA256](https://en.wikipedia.org/wiki/SHA-2)