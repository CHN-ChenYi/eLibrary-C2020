#ifndef BASICTYPE_H_
#define BASICTYPE_H_

#include <stdint.h>

#include "genlib.h"
#include "graphics.h"

typedef struct Book {
  int uid;
  char id[20];
  char title[200];
  char authors[3][200];
  char category[50];
  char press[200];
  char keywords[5][50];
  unsigned int number_on_the_shelf;
  unsigned int available_borrowed_days;
} Book;

typedef enum Identity { NORMAL_USER = 0, ADMINISTRATOR } Identity;
typedef enum Gender { Male = 0, Female } Gender;

typedef struct User {
  int uid;
  char username[50];
  char salt[10];
  uint32_t password[8];
  enum Gender gender;
  char department[200];
  Identity whoami;
  bool verified;
} User;

typedef enum BookStatus { RETURNED = 0, BORROWED } BookStatus;

typedef struct BorrowRecord {
  int uid;
  int book_uid;
  int user_uid;
  char book_name[200];
  char user_name[50];
  char borrowed_date[10];
  BookStatus book_status;
  // if book_status == BORROWED, returned_date stores the expected returned date
  char returned_date[10];
} BorrowRecord;

#endif  // BASICTYPE_H_