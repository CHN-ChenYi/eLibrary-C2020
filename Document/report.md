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

### view

#### main.c

##### Main

* 函数原型

  ```c
  void Main();
  ```

* 功能描述

  调用 template 和 view 的初始化函数，启动程序

* 参数描述

  无

* 返回值描述

  无

* 重要局部变量定义

  无

* 重要局部变量用途描述

  无

* 函数算法描述

  无

#### hash.c

##### Sha256Sum

* 函数原型

  ```c
  void Sha256Sum(uint32_t *const dst, const uint8_t *const src,
                 const uint32_t len);
  ```

* 功能描述

  计算 `src` 中存储的长度为 `len` 的数据的 SHA-256 值并存放在 `dst` 中。将原数据摘要成一个256位的数据。

* 参数描述

  * `dst`: 用以储存哈希值的数组，要求至少 32 字节（也就是 8 个 uint32_t)
  * `src`: 存储原数据的数组
  * `len`: 原数据的长度

* 返回值描述

  无

* 重要局部变量定义

  ```c
  uint32_t n = len / SHA256_BLOCK_SIZE;
  uint32_t m = len % SHA256_BLOCK_SIZE;
  uint8_t cover_data[SHA256_BLOCK_SIZE * 2];
  uint32_t h[8] = { /* details omitted */ };
  ```

* 重要局部变量用途描述

  * `n`: 原数据切分成512位一组后的完整的组数
  * `m`: 原数据切分之后剩余的部分的长度
  * `cover_data`: 原数据切分的多余部分进行补齐之后的数据
  * `h[]`: 哈希值

* 函数算法描述

  先对原数据进行512一组的切分，剩余部分进行补齐并使得新数据的结尾为原数据的比特数（表示形式为64位大端无符号整形）。然后对每一组数据重复 `ChunkProcess` 进行哈希处理，更新 `h[]`。全部处理结束之后 `h[]` 中的数据即为结果。将 `h[]` 中的数据拷贝到 `dst[]` 中

##### ChunkProcess

* 函数原型

  ```c
  static void ChunkProcess(const uint8_t *msg, uint32_t *h);
  ```

* 功能描述

  对 `msg` 中存储的512位数据进行哈希处理并将结果存在 `h[]` 中

* 参数描述

  * `msg`: 512位待进行哈希处理的数据
  * `h[]`: 存储原来的哈希值，新的哈希值也将保存在这里

* 返回值描述

  无

* 重要局部变量定义

  ```c
  static uint32_t k[64] = { /* details omitted */ }
  uint32_t w[64], new_h[8];
  ```

* 重要局部变量用途描述

  * `k[]`: SHA-256 算法中的固定常量
  * `w[]`: 32位的单词。其中前16个单词为传入的512位数据。后48个单词通过固定算法由前面的单词生成
  * `new_h`: 由整个消息（前面生成的单词组 `w[]`）和传入的原哈希值 `h[]` 通过固定算法生成的新的哈希值

* 函数算法描述

  通过各种位运算生成消息并计算其哈希值，详见参考资料

##### RandStr

* 函数原型

  ```c
  void RandStr(char *const dst, const unsigned len);
  ```

* 功能描述

  在 `dst[]` 中生成长度为 `len` 的随机字符串（字典为 ASCII 表（不含扩展表）中的所有可见打印字符）

* 参数描述

  * `dst[]`: 存储随机字符串的数组
  * `len`: 随机字符串的长度

* 返回值描述

  无

* 重要局部变量定义

  无

* 重要局部变量用途描述

  无

* 函数算法描述

  以调用函数时的时间戳为随机种子，逐个生成字符串中的每个字符

#### utility.c

##### InitUtility

* 函数原型

  ```c
  void InitUtility();
  ```

* 功能描述

  初始化日志文件

* 参数描述

  无

* 返回值描述

  无

* 重要局部变量定义

  无

* 重要局部变量用途描述

  无

* 函数算法描述

  无

##### UninitUtility

* 函数原型

  ```c
  void UninitUtility();
  ```

* 功能描述

  关闭日志文件

* 参数描述

  无

* 返回值描述

  无

* 重要局部变量定义

  无

* 重要局部变量用途描述

  无

* 函数算法描述

  无

##### Log

* 函数原型

  ```c
  void Log(char *const msg);
  ```

* 功能描述

  将 `msg[]` 前加上当地时间并写入日志文件

* 参数描述

  `msg[]`: 要写入日志文件的消息

* 返回值描述

  无

* 重要局部变量定义

  ```c
  const time_t cur_time = time(0);
  ```

* 重要局部变量用途描述

  * `cur_time`: 当前时间

* 函数算法描述

  获取当前时间之后先后使用 `loacltime` 函数和 `asctime` 函数将其转换为当地时间的字符串表示，并将字符串结尾的换行符去除。将时间字符串及 `msg[]` 输出到日志文件之中

##### MoveInList

* 函数原型

  ```c
  char *MoveInList(ListNode **const node, List *list, int max_size,
                   bool direction, const char *const list_name,
                   const char *const page_name);
  ```

* 功能描述

  将 `node` 在链表 `list` 中的指向向 `direction`（0为向前，1为向后）方向移动 `max_size` 个节点，并返回相应的日志消息（当前链表名为 `list_name`，当前界面名为 `page_name`）

* 参数描述

  见功能描述

* 返回值描述

  见功能描述

* 重要局部变量定义

  无

* 重要局部变量用途描述

  无

* 函数算法描述

  见功能描述。注意如果移动失败（超过链表结尾或开头）则 `node` 的指向不变并返回报告出错的日志消息

##### ErrorHandle

* 函数原型

  ```c
  bool ErrorHandle(int errno_, int num, ...)
  ```

* 功能描述

  处理数据库操作的返回值，如果数据库操作失败，则绘制历史记录中的上一个页面并显示错误信息

* 参数描述

  * `errno_`: 数据库操作返回的错误码
  * `num`: 可变参数的个数
  * 可变参数: 可以视作成功的错误码

* 返回值描述

  如果为 FALSE 则表示数据库操作成功，如果为 TRUE 则表示数据库操作失败

* 重要局部变量定义

  无

* 重要局部变量用途描述

  无

* 函数算法描述

  见功能描述

##### InitCheck

* 函数原型

  ```c
  bool InitCheck(bool no_user);
  ```

* 功能描述

  验证数据库是否打开以及当前是否有用户登录，检查如果失败则绘制历史记录中的上一个页面并显示错误信息

* 参数描述

  `no_user`: TRUE 表示可以接受没有用户登录

* 返回值描述

  TRUE 表示检查失败（如数据库未打开或用户未登录），FALSE 表示检查没有问题

* 重要局部变量定义

  无

* 重要局部变量用途描述

  无

* 函数算法描述

  见功能描述

##### CmpGreaterBorrowRecordByReturnTime

* 函数原型

  ```c
  bool CmpGreaterBorrowRecordByReturnTime(const void *const lhs,
                                          const void *const rhs)
  ```

* 功能描述

  以归还时间为关键字比较两个指向 BorrowRecord 的指针

* 参数描述

  *  `lhs`: 指向不等号左边的值的指针
  *  `rhs`: 指向不等号右边的值的指针

* 返回值描述

  返回 `lhs` 的归还时间是否大于等于 `rhs`

* 重要局部变量定义

  无

* 重要局部变量用途描述

  无

* 函数算法描述

  无

##### CmpLessBorrowRecordByReturnTime

* 函数原型

  ```c
  bool CmpLessBorrowRecordByReturnTime(const void *const lhs,
                                       const void *const rhs)
  ```

* 功能描述

  以归还时间为关键字比较两个指向 BorrowRecord 的指针

* 参数描述

  *  `lhs`: 指向不等号左边的值的指针
  *  `rhs`: 指向不等号右边的值的指针

* 返回值描述

  返回 `lhs` 的归还时间是否小于等于 `rhs`

* 重要局部变量定义

  无

* 重要局部变量用途描述

  无

* 函数算法描述

  无

##### CmpLessBookById

* 函数原型

  ```c
  bool CmpLessBookById(const void *const lhs, const void *const rhs);
  ```

* 功能描述

  以书号为关键字比较两个指向 Book 的指针

* 参数描述

  *  `lhs`: 指向不等号左边的值的指针
  *  `rhs`: 指向不等号右边的值的指针

* 返回值描述

  返回 `lhs` 的书号是否小于等于 `rhs`

* 重要局部变量定义

  无

* 重要局部变量用途描述

  无

* 函数算法描述

  无

##### CmpLessBookByTitle

* 函数原型

  ```c
  bool CmpLessBookByTitle(const void *const lhs, const void *const rhs);
  ```

* 功能描述

  以书名为关键字比较两个指向 Book 的指针

* 参数描述

  *  `lhs`: 指向不等号左边的值的指针
  *  `rhs`: 指向不等号右边的值的指针

* 返回值描述

  返回 `lhs` 的书名是否小于等于 `rhs`

* 重要局部变量定义

  无

* 重要局部变量用途描述

  无

* 函数算法描述

  无

##### CmpLessBookByAuthor

* 函数原型

  ```c
  bool CmpLessBookByAuthor(const void *const lhs, const void *const rhs);
  ```

* 功能描述

  以第一作者为关键字比较两个指向 Book 的指针

* 参数描述

  *  `lhs`: 指向不等号左边的值的指针
  *  `rhs`: 指向不等号右边的值的指针

* 返回值描述

  返回 `lhs` 的第一作者是否小于等于 `rhs`

* 重要局部变量定义

  无

* 重要局部变量用途描述

  无

* 函数算法描述

  无

##### CmpLessUserById

* 函数原型

  ```c
  bool CmpLessUserById(const void *const lhs, const void *const rhs);
  ```

* 功能描述

  以第一作者为关键字比较两个指向 User 的指针

* 参数描述

  *  `lhs`: 指向不等号左边的值的指针
  *  `rhs`: 指向不等号右边的值的指针

* 返回值描述

  返回 `lhs` 的用户号是否小于等于 `rhs`

* 重要局部变量定义

  无

* 重要局部变量用途描述

  无

* 函数算法描述

  无

##### CmpLessUserByName

* 函数原型

  ```c
  bool CmpLessUserByName(const void *const lhs, const void *const rhs);
  ```

* 功能描述

  以第一作者为关键字比较两个指向 User 的指针

* 参数描述

  *  `lhs`: 指向不等号左边的值的指针
  *  `rhs`: 指向不等号右边的值的指针

* 返回值描述

  返回 `lhs` 的姓名是否小于等于 `rhs`

* 重要局部变量定义

  无

* 重要局部变量用途描述

  无

* 函数算法描述

  无

##### CmpLessUserByDepartment

* 函数原型

  ```c
  bool CmpLessUserByDepartment(const void *const lhs, const void *const rhs);
  ```

* 功能描述

  以第一作者为关键字比较两个指向 User 的指针

* 参数描述

  *  `lhs`: 指向不等号左边的值的指针
  *  `rhs`: 指向不等号右边的值的指针

* 返回值描述

  返回 `lhs` 的部门是否小于等于 `rhs`

* 重要局部变量定义

  无

* 重要局部变量用途描述

  无

* 函数算法描述

  无

##### StrCpy

* 函数原型

  ```c
  void *const StrCpy(void *const str);
  ```

* 功能描述

  深拷贝 `str` 中储存的字符串

* 参数描述

  * `str`: 待拷贝的字符串

* 返回值描述

  * 返回复制出的字符串

* 重要局部变量定义

  * 无

* 重要局部变量用途描述

  * 无

* 函数算法描述

  使用 `malloc` 为返回的字符串分配空间然后 `strcpy` 过去

##### StrLess

* 函数原型

  ```c
  bool StrLess(const void *const lhs, const void *rhs);
  ```

* 功能描述

  比较两个字符串

* 参数描述

  *  `lhs`: 指向不等号左边的值的指针
  *  `rhs`: 指向不等号右边的值的指针

* 返回值描述

  返回 `lhs` 是否小于等于 `rhs`

* 重要局部变量定义

* 重要局部变量用途描述

* 函数算法描述

##### StrSame

* 函数原型

  ```c
  bool StrSame(const void *const lhs, const void *rhs);
  ```

* 功能描述

  比较两个字符串

* 参数描述

  *  `lhs`: 指向不等号左边的值的指针
  *  `rhs`: 指向不等号右边的值的指针

* 返回值描述

  返回 `lhs` 是否等于 `rhs`

* 重要局部变量定义

* 重要局部变量用途描述

* 函数算法描述

##### GetTime

* 函数原型

  ```c
  char *GetTime(time_t dst_tm);
  ```

* 功能描述

  将 time_t 转换成格式为 YYYYMMDD 的字符串。

* 参数描述

  * `dst_tm`: 需要转换的时间戳

* 返回值描述

  返回对应字符串。注意：指针指向静态储存区，请不要 `free`

* 重要局部变量定义

  无

* 重要局部变量用途描述

  无

* 函数算法描述

  无

##### GetBorrowRecordNumberAfter

* 函数原型

  ```c
  int GetBorrowRecordNumberAfter(List *borrow_record, time_t dst_tm);
  ```

* 功能描述

  统计链表 `borrow_record` 中借阅时间在 `dst_tm` 之后的借阅记录的个数

* 参数描述

  * `borrow_record`: 待统计的借阅记录。注意：在传入之前应先按归还时间的降序排序
  * `det_tm`: 要求的时间

* 返回值描述

  满足要求的借阅记录的个数

* 重要局部变量定义

  无

* 重要局部变量用途描述

  无

* 函数算法描述

  遍历整个链表直至归还时间早于要求的时间。（由于调用此函数时链表均已按照归还时间排序过，所以可以这样剪枝）

#### history.c

##### InitHistory

* 函数原型

  ```c
  void InitHistory();
  ```

* 功能描述

  初始化历史记录

* 参数描述

  无

* 返回值描述

  无

* 重要局部变量定义

  无

* 重要局部变量用途描述

  无

* 函数算法描述

  初始化历史记录链表

##### UninitHistory

* 函数原型

  ```c
  void UninitHistory();
  ```

* 功能描述

  关闭历史记录模块

* 参数描述

  无

* 返回值描述

  无

* 重要局部变量定义

  无

* 重要局部变量用途描述

  无

* 函数算法描述

  清空并释放历史记录列表

##### TopHistory

* 函数原型

  ```c
  History *const TopHistory();
  ```

* 功能描述

  返回历史记录模块中栈顶的历史记录（函数只是为了提高代码简洁性，只要编译器优化级别不过于保守，应该会自动内联）

* 参数描述

  无

* 返回值描述

  指向栈顶历史记录的指针

* 重要局部变量定义

  无

* 重要局部变量用途描述

  无

* 函数算法描述

  无

##### PushHistory

* 函数原型

  ```c
  void PushBackHistory(History *const new_history);
  ```

* 功能描述

  向栈中压入新的历史记录

* 参数描述

  `new_history`: 指向新历史记录的指针

* 返回值描述

  无

* 重要局部变量定义

  无

* 重要局部变量用途描述

  无

* 函数算法描述

  直接加入链表末尾，如果链表大小超过设置的历史记录上限，则弹出链表开头的历史记录

##### PopHistory

* 函数原型

  ```c
  void PopBackHistory();
  ```

* 功能描述

  弹出栈顶历史记录

* 参数描述

  无

* 返回值描述

  无

* 重要局部变量定义

  无

* 重要局部变量用途描述

  无

* 函数算法描述

  无

##### ClearHistory

* 函数原型

  ```c
  void ClearHistory();
  ```

* 功能描述

  清空历史记录

* 参数描述

  无

* 返回值描述

  无

* 重要局部变量定义

  无

* 重要局部变量用途描述

  无

* 函数算法描述

  不断调用 `PopBackHistory` 直至栈为空

##### FreeHistory

* 函数原型

  ```c
  void FreeHistory(void *const history_);
  ```

* 功能描述

  释放历史记录的栈中的元素的内存

* 参数描述

  * `history_`: 指向要释放的历史记录的指针

* 返回值描述

  无

* 重要局部变量定义

  无

* 重要局部变量用途描述

  无

* 函数算法描述

  释放历史记录中动态申请出来的内存（如链表、字符串等）

##### ReturnHistory

* 函数原型

  ```c
  void ReturnHistory(ListNode *go_back_to, char *msg);
  ```

* 功能描述

  回滚到指定的历史记录，并输出相应的日志信息

* 参数描述

  * `go_back_to`: 指向要回滚到的历史记录
  * `msg`: 要输出的日志信息

* 返回值描述

  无

* 重要局部变量定义

  无

* 重要局部变量用途描述

  无

* 函数算法描述

  根据要回滚到的历史记录的页面不同绘制对应的页面，如果栈被清空了，则绘制欢迎界面

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
