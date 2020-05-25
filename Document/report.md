# 1 大程序简介

## 1.1 选题背景及意义
**图书管理系统**是生活中相当常见的管理系统之一，但这些管理系统还是有其缺陷，如无法准确快速的搜索到数据、整体设计复杂繁琐、体积庞大不适用某些小专案。
因此本次我组选题为**图书管理系统**，其一是希望开发一个简易好上手、快速的管理系统；其二是图书管理系统是一个较为整齐且分工明确的项目，且涉及的各个模块实作跨度较大(绘制/排序/查找)，能接触不同领域的开发。  
我组希望透过这次大程实现简易管理系统，能更了解C语言模块化程序设计。
## 1.2 目标要求
* 能更熟悉运用`libgraphics`函数库  
* 能更熟悉文件在C语言中的实现及处理  
* 理解并掌握基本的数据处理，对于往后数据库的学习打下基础
* 理解管理系统的原型    
* 学习如何与他人分工协作  

## 1.3 术语说明

### 深拷贝(deep copy)
指在拷贝时，完全拷贝一个对象的值及其所含的子对象的值到目标对象，深拷贝后将有两个对象出现，并为不同个体，彼此间占用不同内存空间，不互相影响。  
与其相对的是**浅拷贝(shallow copy)**，浅拷贝仅仅传递或赋予指针，实际对象数并没有增加。
### 哈希(hash)
是从一笔数据中，透过一个**哈希函数**，来创建较为简短，具有独特性的**哈希值**（通常以字母和数字）的方法。如此一来，数据便有了统一的代表格式，也可利用**哈希值**来取得原始数据。
除此之外，根据哈希函数的设计，其碰撞(产生重复哈希值)概率、安全性也不同。哈希目前广泛应用於电脑技术中，如数字签名等。
### 栈(stack)
一种常见的数据结构，栈中数据保持先进后出(FILO)的次序。  
### 内联函数(inline functions)
内联函数是一种使用关键词`inline`宣告的函数，用来建议编译器对这些函数进行内联拓展，也就是说将这些函数的函数体取代每一处该调用函数的地方，用以节省调用函数的时间开支。  
内联函数通常为简短但是常调用的函数。
### 模型(model)
模型为数据结构在编程时的抽象表示，与其相对的是**实例(instance)**，代表实际在内存中的数据结构。
# 2 功能需求分析

### 编辑
* 新增
* 复制粘贴 
* 查询 
* 修改、删除
### 检索
* 查找图书
* 查找用户
### 统计
* 分类统计
### 排序
* 根据书号排序
* 根据图书名排序
* 根据作者排序
### 存取
* 数据在内存与文件的转换


# 3 程序开发设计

## 3.1 总体架构设计

## 3.2 功能模块设计

## 3.3 数据结构设计

## 3.4 源代码文件组织设计

## 3.5 函数设计描述

### model

#### model.c(主要函数)
##### OpenDBConnection
* 函数原型
  ```c
  int OpenDBConnection(const char* filename, Model model);
  ```
* 功能描述  
用以初始化，打开数据库文件以供读写，并加载其内容至链表。
* 参数描述  
`filename` - 数据库文件存放地点  
`model` - 数据库的模型(BOOK, USER, BORROWRECORD...)  
* 返回值描述  
`int`, 为`DBErrno`(数据库错误码)
* 函数算法描述  
此函数又以两个辅助函数实现，分别为`DBOpen()`和`DBInit()`，其中`DBOpen()`实现打开数据库文件的
具体功能，而`DBInit()`实现打开文件后将其内容加载至链表的功能。此函数首先以`DBExists()`判断数据库模型是否存在后，先后呼叫这两个函数。
详见`model.c`(辅助函数)中的`DBOpen()`和`DBInit()`。

##### CloseDBConnection
* 函数原型
  ```c
  int CloseDBConnection(Model model);
  ```
* 功能描述  
用以初始化，打开数据库文件以供读写，并加载其内容至链表。
* 参数描述  
`model` - 数据库的模型(BOOK, USER, BORROWRECORD...)
* 返回值描述  
`int`, 为`DBErrno`(数据库错误码)
* 函数算法描述  
此函数又以两个辅助函数实现，分别为`DBClose()`和`DBUninit()`，其中`DBClose()`实现关闭数据库文件的具体功能，而`DBUninit()`实现将链表数据写入文件的功能。此函数首先以`DBExists()`判断数据库模型是否存在后，先后呼叫这两个函数。
详见`model.c`(辅助函数)中的`DBClose()`和`DBUninit()`。

##### Create
* 函数原型
  ```c
  int Create(void* handle, Model model);
  ```
* 功能描述  
在数据库链表中插入一笔新数据。
* 参数描述  
`handle` - 指向一个数据模型实例，代表即将插入的数据。  
`model` - 数据库的模型(BOOK, USER, BORROWRECORD...)   
* 返回值描述  
`DB_FAIL_ON_CREATE` - 代表插入数据失败。  
`DB_SUCCESS` - 代表插入数据成功。  
* 函数算法描述  
先判断当前实例属于哪个模型，再执行插入:深拷贝一份实例，将其插入链表，并将数据库`pk`和`size`的值
各加`1`。关于每个模型的拷贝函数，详见`utils.c`中的`*Copy()`。

##### GetById
* 函数原型
  ```c
  int GetById(void* handle, unsigned int id, Model model);
  ```
* 功能描述  
以`id`查找数据并返回。
* 参数描述  
`handle` - 指向一个数据模型实例，代表即将取得的数据。  
`id` - 实例的id。  
`model` - 数据库的模型(BOOK, USER, BORROWRECORD...)   
* 返回值描述  
`DB_NOT_FOUND` - 找不到数据库。  
`DB_NOT_EXISTS` - 找不到实例。  
`DB_SUCCESS` - 代表查找数据成功。  
* 函数算法描述  
先判断当前实例属于哪个模型，再执行`Find()`查找，然后深拷贝一份实例。关于查找函数，详见`model.c`(辅助函数)的`Find()`。关于每个模型的拷贝函数，详见`utils.c`中的`*Copy()`。


##### Filter
* 函数原型
  ```c
  int Filter(List* list_handle, String queries, Model model);
  ```
* 功能描述  
批量查找数据。
* 参数描述  
`list_handle` - 指向一个数据模型实例链表，代表应获得的数据。  
`queries` - 查找请求。   
`model` - 数据库的模型(BOOK, USER, BORROWRECORD...)   
* 返回值描述  
`DB_NOT_FOUND` - 数据库不存在。  
`DB_SUCCESS` - 代表查找数据成功。  
* 重要局部变量定义
  ```c
  List** data = &DBs[model].data;
  ```
* 重要局部变量用途描述  
`data` - 指向数据库链表在内存中的位置。  
* 函数算法描述  
先判断当前实例属于哪个模型，遍历`*data`的同时，调用相对应`*Filter()`函数判断其是否符合请求，若是，深拷贝一份并插入链表`*list_handle`。关于`*Filter()`，详见`utils.c`(辅助函数)。

##### GetDBSize
* 函数原型
  ```c
  int GetDBSize(Model model, unsigned int *size)；
  ```
* 功能描述  
取得数据库大小。
* 参数描述  
`model` - 数据库的模型(`BOOK`, `USER`, `BORROWRECORD`...)  
`size` - 指针指向即将取得数据库大小的变量  
* 返回值描述  
`DB_NOT_FOUND` - 找不到数据库  
`DB_SUCCESS` - 代表成功。  
* 函数算法描述  
将数据库`size`的值赋给`*size`。
##### GetNextPK
* 函数原型
  ```c
  int GetNextPK(Model model, unsigned int *pk)；
  ```
* 功能描述  
取得下一个尚未使用的`pk`。
* 参数描述  
`model` - 数据库的模型(`BOOK`, `USER`, `BORROWRECORD`...)  
`pk` - 指针指向即将取得数据库`pk`的变量  
* 返回值描述  
`DB_NOT_FOUND` - 找不到数据库  
`DB_SUCCESS` - 代表成功。  
* 函数算法描述  
将数据库`pk`的值赋给`*pk`。

##### Update
* 函数原型
  ```c
  int Update(void* handle, unsigned int id, Model model);
  ```
* 功能描述  
更新链表中的数据实例。
* 参数描述  
`handle` - 指向一个数据模型实例，代表即将插入的数据。  
`id` - 需更新实例的id。  
`model` - 数据库的模型(BOOK, USER, BORROWRECORD...)   
* 返回值描述  
`DB_NOT_FOUND` - 找不到数据库。  
`DB_NOT_EXISTS` - 找不到实例。  
`DB_SUCCESS` - 代表更新数据成功。   
* 重要局部变量定义
  ```c
  ListNode* target
  ```
* 重要局部变量用途描述  
`target`- 指向需更新的实例所属的链表节点。  
* 函数算法描述
使用`Find()`根据`id`查找实例，找到后，深拷贝一份新实例至原实例。关于查找函数，详见`model.c`(辅助函数)的`Find()`。

##### Delete
* 函数原型
  ```c
  int Delete(unsigned int id, Model model);
  ```
* 功能描述  
删除链表中的数据实例。
* 参数描述  
`handle` - 指向一个数据模型实例，代表即将插入的数据。  
`id` - 需更新实例的id。  
`model` - 数据库的模型(BOOK, USER, BORROWRECORD...)   
* 返回值描述  
`DB_FAIL_ON_DELETE` - 删除数据失败。  
`DB_SUCCESS` - 代表删除数据成功。  
* 重要局部变量定义
  ```c
  ListNode* target
  ```
* 重要局部变量用途描述  
`target`- 指向需删除的实例所属的链表节点。  
* 函数算法描述
使用`Find()`根据`id`查找实例，找到后，删除此实例及其相对应节点，并将数据库`size`减`1`。关于查找函数，详见`model.c`(辅助函数)的`Find()`。

#### model.c(辅助函数)
##### DBExists
* 函数原型
  ```c
  inline int DBExists(Model model);
  ```
* 功能描述  
判断数据库模型是否存在。
* 参数描述  
`model` - 整数，代表一个数据库模型。
* 返回值描述  
返回`1`如果数据库模型存在；反之则`0`。
* 函数算法描述  
代表数据库模型的整数必须大于等于0且小于等于宏定义`N_MODEL`才为合法数据库模型，当前`N_MODEL`为3。

##### DBOpen
* 函数原型
  ```c
  int DBOpen(const char* filename, Model model);
  ```
* 功能描述  
打开数据库文件。
* 参数描述  
`filename` - 数据库文件存放地点  
`model` - 数据库的模型(`BOOK`, `USER`, `BORROWRECORD`...)  
* 返回值描述  
`DB_SUCCESS` - 代表数据库文件打开成功。
* 重要局部变量定义  
  ```c
  FILE* database;
  ```
* 重要局部变量用途描述  
`database`- 数据库文件object。
* 函数算法描述  
首先以读+更新形式打开数据库文件，如果打不开或数据库文件不存在，则以只写形式覆盖或新增数据库文件。

##### DBClose
* 函数原型
  ```c
  int DBClose(Model model);
  ```
* 功能描述  
关闭数据库文件。
* 参数描述  
`model` - 数据库的模型(`BOOK`, `USER`, `BORROWRECORD`...)
* 返回值描述  
`DB_NOT_CLOSE` - 代表数据库没有正确关闭。  
`DB_SUCCESS` - 代表数据库文件关闭成功。  
* 函数算法描述  
用`fclose()`关闭文件。

##### DBInit
* 函数原型
  ```c
  int DBInit(Model model);
  ```
* 功能描述  
加载数据库文件至链表。
* 参数描述  
`model` - 数据库的模型(`BOOK`, `USER`, `BORROWRECORD`...)
* 返回值描述   
`DB_ENTRY_EMPTY` - 读取数据时出错导致数据不完整。  
`DB_FAIL_ON_INIT` - 加载数据时出错。  
`DB_SUCCESS` - 代表数据库加载成功。  
* 重要局部变量定义
  ```c
  FILE** database = &DBs[model].database;
  List** data = &DBs[model].data;
  ```
* 重要局部变量用途描述  
`database`- 指向数据库object在内存中的位置。  
`data` - 指向数据库链表在内存中的位置。  
* 函数算法描述
新建链表，再将数据按照格式从`*database`读入字串，读第一遍时若字串为空，代表这是一个新数据库，将数据库的`pk`和`size`初始化为0；若非空，则读入`pk`和`size`。之后的读入皆为读入数据，将读入的字串利用`StringToModel()`转为数据库模型实例，再将实例插入到链表`*data`中。
关于转换，详见`utils.c`的`StringToModel()`。

##### DBUninit
* 函数原型
  ```c
  int DBUninit(Model model);
  ```
* 功能描述  
将数据库链表写入数据库文件中并销毁释放链表。
* 参数描述  
`model` - 数据库的模型(`BOOK`, `USER`, `BORROWRECORD`...)
* 返回值描述   
`DB_NOT_FOUND` - 数据库不存在。  
`DB_FAIL_ON_UNINIT` - 写入文件时出错。  
`DB_SUCCESS` - 代表数据库文件写入成功。  
* 重要局部变量定义
  ```c
  FILE* database = DBs[model].database;
  List* data = DBs[model].data;
  ```
* 重要局部变量用途描述  
`database`- 指向数据库object。  
`data` - 指向数据库链表。  
* 函数算法描述
首先写入数据库`pk`和`size`(第一行)，随后遍历链表，将其连结的数据模型实例用`ModelToString()`转换为字串并写入文件，等所有数据都处理完后，销毁链表。关于转换，详见`utils.c`的`ModelToString()`。

##### Find
* 函数原型
  ```c
  int Find(ListNode** target, unsigned int id, Model model);
  ```
* 功能描述  
遍历链表，根据id找到对应数据模型实例。
* 参数描述  
`target` - 指针指向链表节点存取位置。  
`id` - 实例的id。  
`model` - 数据库的模型(`BOOK`, `USER`, `BORROWRECORD`...)  
* 返回值描述  
`DB_NOT_EXISTS` - 此实例不存在。  
`DB_SUCCESS` - 代表成功找到。  
* 函数算法描述  
判断数据实例为哪个数据模型，再简单`O(n)`遍历相对应的链表。

#### utils.c
##### SaveStrCpy
* 函数原型
  ```c
  int SaveStrCpy(char* t, const char* s);
  ```
* 功能描述  
避免字串为空指针，多了检查的安全拷贝。
* 参数描述  
  `t` - 指针指向拷贝目标  
  `s` - 指针指向拷贝来源  
* 返回值描述  
  `DB_FAIL_ON_FETCHING` - 拷贝失败  
  `DB_SUCCESS` - 拷贝成功  
* 函数算法描述  
先判断字串是否为空指针，再执行拷贝。
##### *Copy
* 函数原型  
  ```c
  int BookCopy(Book* destination, Book* source);
  int UserCopy(User* destination, User* source);
  int RecordCopy(BorrowRecord* destination, BorrowRecord* source);
  ```
* 功能描述  
深拷贝一份实例。
* 参数描述  
`destination` - 拷贝目标  
`source` - 拷贝来源  
* 返回值描述  
  `DB_FAIL_ON_FETCHING` - 拷贝失败  
  `DB_SUCCESS` - 拷贝成功 
* 函数算法描述  
将来源实例结构中的每个变量深拷贝一份到目标实例中。
##### Cmp
* 函数原型  
  ```c
  int Cmp(const char* str1, const char* str2, int insensitive, int equal);
  ```
* 功能描述  
  根据`insensitive`和`equal`判断两字串是否符合关系。
* 参数描述  
  `str1` - 字串1  
  `str2` - 字串2  
  `insenstive` - 是否模糊  
  `equal` - 是否相等  
* 返回值描述  
  回传`1`如果两字串符合关系；反之，回传`0`。
* 函数算法描述  
  如果`insensitive`为真，`str2`在`str1`中则符合关系。如果`insensitive`为假且`equal`为真，两者需相等才符合关系；若`equal`为假，则两者需不相等。
##### *Filter
* 函数原型  
  ```c
  int BookFilter(Book* p_b, String queries);
  int UserFilter(User* p_u, String queries);
  int RecordFilter(BorrowRecord* p_r, String queries);
  ```
* 功能描述  
判断一实例是否符合请求。
* 参数描述  
`p_*` - 指针指向实例。  
`queries` - 请求。  
* 返回值描述  
`int`。如果符合请求，回传`1`；反之回传`0`。
* 重要局部变量定义  
  ```c
  char* property;
  char* para;
  int insensitive;
  int equal;
  ```
* 重要局部变量用途描述  
`property` - 属性，代表实例结构中一个变量。  
`para` - 参数，代表实例结构中变量的值。  
`insensitive` - 是否模糊查找。  
`equal` - 是否相等。  
* 函数算法描述  
使用`strtok()`，按照先`property`后`para`的次序，切割`queries`，得到`property`和`para`的值，再判断`property`是实例结构中的哪个变量，并用`para`与其进行相对应比较。关于比较，详见`Cmp()`。
##### Slice
* 函数原型  
  ```c
  int Slice(const char* str, char* slice, int* pos);
  ```
* 功能描述  
类似`strtok()`的自定义字串切割函数。
* 参数描述  
`str` - 待切割字串。  
`slice` - 切割后的一小段字串切片。  
`pos` - 待切割字串起始点。  
* 返回值描述  
`DB_FAIL_ON_INIT` - 切割时发生错误。  
`DB_SUCCESS` - 切割成功。
* 函数算法描述  
从`pos`开始遍历字串，并将其字元新增至切片，遇到目标字元`;`时停止遍历。
##### StringTo*
* 函数原型  
  ```c
  int StringToBook(Book* p_b, String str);
  int StringToUser(User* p_u, String str);
  int StringToRecord(BorrowRecord* p_r, String str);
  ```
* 功能描述  
  将字串转为指定模型实例。
* 参数描述  
  `p_*` - 指针指向目标实例。  
  `str` - 待转换字串。  
* 返回值描述  
  `DB_FAIL_ON_FETCHING` - 在途中拷贝失败，转换失败。  
  `DB_SUCCESS` - 转换成功。  
* 重要局部变量定义  
  ```c
  int pos;
  ```
* 重要局部变量用途描述  
  `pos` - 字串切割起始点。
* 函数算法描述  
使用切割函数`Slice()`按照数据格式切割字串，得到实例结构变量的值，将其赋予实例结构中的变量。关于切割函数，详见`Slice()`。
##### StringToModel
* 函数原型  
  ```c
  int StringToModel(void** handle, Model model, String str);
  ```
* 功能描述  
  将字串转为模型实例。
* 参数描述  
  `handle` - 指针指向目标模型实例存取位置。  
  `model` - 数据库的模型(`BOOK`, `USER`, `BORROWRECORD`...)  
  `str` - 待转换字串。  
* 返回值描述  
`DB_SUCCESS` - 代表转换成功。 
* 函数算法描述  
先判断当前实例属于的数据模型，再调用相应转换函数`StringTo*()`，详见`StringTo*()`。
##### ModelToString
* 函数原型  
  ```c
  int ModelToString(void* handle, Model model, char* p_str);
  ```
* 功能描述  
将数据实例转换为字串。
* 参数描述  
  `handle` - 指针指向待转换模型实例。  
  `model` - 数据库的模型(`BOOK`, `USER`, `BORROWRECORD`...)  
  `str` - 目标字串。  
* 返回值描述  
`0`，代表成功。
* 重要局部变量定义  
  ```c
  char p_str_2[1000];
  ```
* 重要局部变量用途描述  
`p_str2` - 实例结构中的变量转换而成的部分字串。
* 函数算法描述  
先判断当前实例属于的数据模型，再将实例中的变量一一转换为字串`p_str2`，并将`p_str2`接在`p_str`后面，最后`p_str`会成为完整的字串。

### template

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

## 5.3 编码规范

## 5.4 合作总结

## 5.5 收获感言

# 6 参考文献资料
