#ifndef BASICTYPE_H_
#define BASICTYPE_H_

#include "graphics.h"
#include "genlib.h"

typedef char* String;

typedef struct Book {
  int uid;
  String id;
  String title;
  String authors[3];
  String category;
  String press;
  String keywords[5];
  LibImage cover;
  unsigned int number_on_the_shelf;
  unsigned int available_borrowed_days;
} Book;

typedef enum Identity { NORMAL_USER = 0, ADMINISTRATOR } Identity;

typedef struct User {
  int uid;
  String username;
  String password;
  String gender;
  String department;
  Identity whoami;
  bool verified;
} User;

typedef enum BookStatus { RETURNED = 0, BORROWED } BookStatus;

typedef struct BorrowRecord {
  int uid;
  int book_uid;
  int user_uid;
  String book_name;
  String user_name;
  String borrowed_date;
  BookStatus book_status;
  // if book_status == BORROWED, returned_date stores the expected returned date
  String returned_date;
} BorrowRecord;

#endif  // BASICTYPE_H_