#ifndef MODEL_H_
#define MODEL_H_

#include "basictype.h"
typedef struct Book Book;
typedef struct User User;
typedef struct BorrowRecord BorrowRecord;
typedef StringList IdList;

enum DBErrno {
      DB_SUCCESS,

      DB_NOT_FOUND,
      DB_NOT_OPEN,
      DB_FAIL_ON_WRITING,
      DB_NOT_EXISTS,
      DB_ALREADY_EXISTS,

      DB_FAIL
};


// Open and close the DB.
/*
  Open(Close)DBConnection

  It's used to open(close) the DB.
  You should call the open function when you want to access to the DB, then close it when you don't need to.

  Parameters:
  none

  Return value:
  DBErrno
 */
int OpenDBConnection(void);
int CloseDBConnection(void);
// Create
/*
  Create*

  It's used to create data in the DB

  Parameters:

  (book,user,record)
  The pointer pointing to the actual memory that will be used to stord data.

  Return value:
  1 if error appears, 0 otherwise.
 */
// return 1 if error appears, 0 otherwise.
int CreateBook(Book* book);
int CreateUser(User* user);
int CreateBorrowRecord(BorrowRecord* record);

// Request
// For many functions in this section, the first parameter is the pointer pointing to the actual instance in memory(the item you want) and will be free when the DB is closed in order to prevent memory leakage. As a result, you should not access to these instances after the DB is closed.
/*
  Get*ById
  It's used to get data by it's id. The faster way to retrieve data.

  Parameters:

  (book,user,record)
  The pointer pointing to the actual memory that will be used to stord data.

  *_id
  *'s id.

  Return value:
  DBErrno
 */

int GetBookById(Book* book, String book_id);
int GetUserById(User* user, String user_id);
int GetBorrowRecordById(BorrowRecord* record, String record_id);

/* int GetBooksByTitle(IdList* book_list, String title); */
/* int GetBooksByUID(IdList* book_list, String uid); */
/* int GetBooksByKeyword(IdList* book_list, String keyword); */
/* int GetBooksByAuthor(IdList* book_list, String author); */
/* int GetBorrowRecordsByUID(IdList* record_list, String user_id); */
/* int GetBorrowRecordByBID(IdList* record_list, String book_id); */

/*
 Filter*

 Get data by one or more arguments.

 Parameters:

 *_list
 The pointer pointing to the list used to store a series of data

 queries

 Formatting your queries like this:
 name1=value1&name2=value2&name3=value3...
 Ex 1.
 Assume we want to get users that are male:
 "gender=male"

 Ex 2.
 Assume we want to get books that are published by "Cambridge" on March 17, 2013, and have one of the keyword "Math":
 "press=Cambridge&keyword=Math&published_date=2013-03-17"

 Ex 3.
 Assume we want to filter books by two keywords, "Java" and "Javacript"
 "keyword=Java&keyword=JavaScript"
 Notice that in the above example, the filter is case-sensitive.

 You can also add a ;(semicolon) in front of the = for case-insensitive search.

 Ex 4.
 Assume we want to get books that are published by O Reilly and related to C(so you might get books about C/C++/C#/Objective-C!)
 "press=O Reilly&keyword;=C"

 Return value:
 DBErrno
*/
int FilterBooks(IdList* book_list, String queries);
int FilterUsers(IdList* user_list, String queries);
int FilterBorrowRecords(IdList* record_list, String queries);

/*
  Get*NextPK
  It's used to get The smallest prime key that isn't mapped to a row in database.

  Parameter:
  none

  Return value:
  -1 if error appears, the prime key, unsigned int
 */


int GetBookNextPK(void);
int GetUserNextPK(void);
int GetBorrowRecordNextPK(void);

// Update
/*
  Update*
  It's used to perform update operation on a row in database.

  Parameter:
  *_id

  Return value:
  DBErrno
 */

int UpdateBook(String book_id);
int UpdateUser(String user_id);
int UpdateBorrowRecord(String record_id);

// Delete
/*
  Delete*
  It's used to perform delete operation on a row in database.

  Parameter:
  *_id

  Return value:
  DBErrno
*/

int DeleteBook(String book_id);
int DeleteUser(String record_id);
int DeleteBorrowRecord(String record_id);
#endif // MODEL_H_
