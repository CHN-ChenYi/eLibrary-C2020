#ifndef UTILS_H_
#define UTILS_H_
#include"basictype.h"
#include"model.h"

typedef char* String;
// Copy
int BookCopy(Book* destination, Book* source);
int UserCopy(User* destination, User* source);
int RecordCopy(BorrowRecord* destination, BorrowRecord* source);

// Filter
int BookFilter(Book* p_b, String queries);
int UserFilter(User* p_u, String queries);
int RecordFilter(BorrowRecord* p_r, String queries);

// StringToModel
int StringToModel(void** handle, Model model, String str);

// ModelToString
int ModelToString(void* handle, Model model, char* p_str);

#endif