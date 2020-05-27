#ifndef UTILS_H_
#define UTILS_H_

#include <stdio.h>

#include "basictype.h"

enum DBErrno {
  DB_SUCCESS = 0,

  DB_NOT_FOUND,
  DB_NOT_OPEN,
  DB_NOT_CLOSE,
  DB_NOT_EXISTS,
  DB_FAIL_ON_INIT,
  DB_FAIL_ON_UNINIT,
  DB_FAIL_ON_FETCHING,
  DB_FAIL_ON_WRITING,
  DB_FAIL_ON_CREATE,
  DB_FAIL_ON_UPDATE,
  DB_FAIL_ON_DELETE,
  DB_FAIL_ON_GETTING_PROPERTIES,  // for getting pk and size.
  DB_ALREADY_EXISTS,
  DB_ENTRY_EMPTY,

  DB_FAIL
};
typedef enum Model { BOOK = 0, USER, BORROWRECORD } Model;
typedef char* String;

// Copy
int BookCopy(Book* destination, Book* source);
int UserCopy(User* destination, User* source);
int RecordCopy(BorrowRecord* destination, BorrowRecord* source);

// Filter
int BookFilter(Book* p_b, String queries);
int UserFilter(User* p_u, String queries);
int RecordFilter(BorrowRecord* p_r, String queries);

#endif
