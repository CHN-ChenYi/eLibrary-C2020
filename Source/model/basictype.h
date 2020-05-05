#ifndef BASICTYPE_H_
#define BASICTYPE_H_

#include "graphics.h"

typedef char* String;

typedef struct Book {
  int uid;
  String id;
  String title;
  String authors[3];
  String press;
  String keywords[5];
  LibImage cover;
  unsigned int number_on_the_shelf;
  unsigned int available_borrowed_days;
} Book;

typedef enum Identity { NORMAL_USER = 0, ADMINISTRATOR = 1 } Identity;

typedef struct User {
  int uid;
  String username;
  String password;
  String gender;
  String department;
  Identity whoami;
} User;

typedef struct BorrowRecord {
  int uid;
  String book_uid;
  String user_uid;
  String borrowed_date;
  String expected_return_date;
  String returned_date;
} BorrowRecord;

#endif  // BASICTYPE_H_
