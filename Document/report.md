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
