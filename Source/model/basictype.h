#ifndef BASICTYPE_H_
#define BASICTYPE_H_

typedef char* String;
typedef char** StringList; // To be replaced to actual list type.

struct Book{
  String book_id;
  String title;
  StringList keywords;
  StringList authors;
  String press;
  String published_date;
  unsigned int available_borrowed_days;
};

enum Identity {
               NORMAL_USER = 0, ADMINISTRATOR = 1
};

struct User{
  String user_id;
  String name;
  String gender;
  String department;
  enum Identity whoami;

};

struct BorrowRecord{
  String record_id;
  String book_id;
  String user_id;
  String borrowed_date;
  String returned_date;
};

#endif // BASICTYPE_H_
