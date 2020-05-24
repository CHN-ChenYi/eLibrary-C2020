#ifndef UTILITY_H_
#define UTILITY_H_

#include "list.h"

void InitUtility();
void UninitUtility();

void Log(char *const msg);
char *MoveInList(ListNode **const node, List *list, int max_size,
                 bool direction, const char *const list_name,
                 const char *const page_name);
// FALSE for success，num 是可变参数的个数，可变参数是可以接受的报错
bool ErrorHandle(int errno_, int num, ...);
// FALSE for success, no_user 为 true 表示可以接受未登录用户
bool InitCheck(bool no_user);

bool CmpGreaterBorrowRecordByReturnTime(const void *const lhs,
                                        const void *const rhs);
bool CmpLessBorrowRecordByReturnTime(const void *const lhs,
                                     const void *const rhs);
bool CmpById(const void *const lhs, const void *const rhs);
bool CmpByTitle(const void *const lhs, const void *const rhs);
bool CmpByAuthor(const void *const lhs, const void *const rhs);
bool CmpLessUserById(const void *const lhs, const void *const rhs);
bool CmpLessUserByName(const void *const lhs, const void *const rhs);
bool CmpLessUserByDepartment(const void *const lhs, const void *const rhs);

void *const StrCpy(void *const str);
bool StrLess(const void *const lhs, const void *rhs);
bool StrSame(const void *const lhs, const void *rhs);

// in format of YYYYMMDD
char *GetTime(time_t tm); // TODO:(TO/GA) 在文档中添加
// 传入的链表均为归还时间的降序
int GetBorrowRecordNumberAfter(List *borrow_record,
                               time_t dst_tm);  // TODO:(TO/GA) 在文档中添加

#endif  // !UTILITY_H_
