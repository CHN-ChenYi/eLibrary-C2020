#ifndef MODEL_H_
#define MODEL_H_

#include "basictype.h"
typedef struct Book Book;
typedef struct User User;
typedef struct BorrowRecord BorrowRecord;
typedef StringList IdList;
// Create
// return 1 if error appears, 0 otherwise.
int CreateBook(Book* book);
int CreateUser(User* user);
int CreateBorrowRecord(BorrowRecord* borrow_record);
// Request
// return NULL if error appears.
Book* GetBookById(String book_id);
User* GetUserById(String user_id);
BorrowRecord* GetBorrowRecordById(String record_id);
IdList* GetBooksByTitle(String title);
IdList* GetBooksByUser(String user_id);
IdList* GetBooksByKeyword(String keyword);
IdList* GetBooksByAuthor(String author);
IdList* GetBorrowRecordsByUser(String user_id);
IdList* GetBorrowRecordByBook(String book_id);
// Update
// return 1 if error appears, 0 otherwise.
int UpdateBook(String book_id);
int UpdateUser(String user_id);
int UpdateBorrowRecord(String record_id);
// Delete
// return 1 if error appears, 0 otherwise.
int DeleteBook(String book_id);
int DeleteUser(String book_id);
int DeleteBorrowRecord(String record_id);
#endif // MODEL_H_
