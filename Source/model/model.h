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

enum Model{
      Book,
      User,
      BorrowRecord
};

struct DB{


};
typedef struct DB DB
// Open and close the DB.
/*
  Open(Close)DBConnection

  It's used to open(close) the DB.
  You should call the open function when you want to access to the DB, then close it when you don't need to.

  Parameters:
  struct DB pointer

  Return value:
  DBErrno
 */
int OpenDBConnection(DB* db);
int CloseDBConnection(DB* db);
// Create
/*
  Create

  It's used to create data in the DB

  Parameters:
  db - pointer pointing to struct DB
  instance - pointer pointing to the model instance(book,user,record,etc...)
  model - see enum Model

  Return value:
  1 if error appears, 0 otherwise.
 */
// return 1 if error appears, 0 otherwise.
int Create(DB* db, void* instance, Model model)


// Request
// For many functions in this section, the first parameter is the pointer pointing to the actual instance in memory(the item you want) and will be free when the DB is closed in order to prevent memory leakage. As a result, you should not access to these instances after the DB is closed.
/*
  GetById
  It's used to get data by it's id. The faster way to retrieve data.

  Parameters:

  db - pointer pointing to struct DB
  instance - pointer pointing to the model instance(book,user,record,etc...)
  id - *_id
  model - see enum Model

  Return value:
  DBErrno
 */
int GetById(DB* db, void* instance, String id, Model model);

/*
 Filter

 Get data by one or more arguments.

 Parameters:

 db - pointer pointing to struct DB
 instance - pointer pointing to the model instance(book,user,record,etc...)
 model - see enum Model

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
int Filter(DB* db, void* list, String queries, Model model)

/*
  GetNextPK
  It's used to get The smallest prime key that isn't mapped to a row in database.

  Parameter:
  db - pointer pointing to struct DB
  model - see enum Model

  Return value:
  -1 if error appears, the prime key, unsigned int
 */

int GetNextPK(DB* db, Model model);

// Update
/*
  Update*
  It's used to perform update operation on a row in database.

  Parameter:
  db - pointer pointing to struct DB
  instance - pointer pointing to the model instance(book,user,record,etc...)
  id  - *_id
  model - see enum Model

  Return value:
  DBErrno
 */

int Update(DB* db, void* instance, String id, Model model);

// Delete
/*
  Delete*
  It's used to perform delete operation on a row in database.

  Parameter:
  db - pointer pointing to struct DB
  id - *_id
  model - see enum Model

  Return value:
  DBErrno
*/

int Delete(DB* db, String id, Model model);
#endif // MODEL_H_
