#ifndef MODEL_H_
#define MODEL_H_

#include "basictype.h"
typedef char* String;

enum DBErrno {
  DB_SUCCESS = 0,

  DB_NOT_FOUND,
  DB_NOT_OPEN,
  DB_NOT_CLOSE,
  DB_NOT_EXISTS,
  DB_FAIL_ON_INIT,
  DB_FAIL_ON_FETCHING,
  DB_FAIL_ON_WRITING,
  DB_FAIL_ON_CREATE,
  DB_FAIL_ON_UPDATE,
  DB_FAIL_ON_DELETE,
  DB_FAIL_ON_GETTING_PROPERTIES, // for getting pk and size.
  DB_ALREADY_EXISTS,
  DB_ENTRY_EMPTY,

  DB_FAIL
};
typedef enum Model { BOOK = 0, USER, BORROWRECORD } Model;

typedef struct DB {
  // described in
  // https://www.hwaci.com/sw/sqlite/c3ref/open.html#urifilenameexamples
  String filename;
} DB;
// Open and close the DB.
/*
  Open(Close)DBConnection

  It's used to open(close) the DB.
  You should call the open function when you want to access to the DB, then
  close it when you don't need to.

  Parameters:
  struct DB pointer
  Model(book, user, borrowrecord)

  Return value:
  DBErrno
 */
int OpenDBConnection(DB* db, Model model);
int CloseDBConnection(DB* db, Model model);
// Create
/*
  Create

  It's used to create data in the DB

  Parameters:
  db - pointer pointing to struct DB
  handle - pointer pointing to the handle (book,user,record,etc...)
  model - see enum Model

  Return value:
  1 if error appears, 0 otherwise.
 */
// return 1 if error appears, 0 otherwise.
int Create(DB* db, void* handle, Model model);

// Request
// For many functions in this section, the first parameter is the pointer
// pointing to the actual instance in memory(the item you want) and will be free
// when the DB is closed in order to prevent memory leakage. As a result, you
// should not access to these instances after the DB is closed.
/*
  GetById
  It's used to get data by it's id. The faster way to retrieve data.

  Parameters:

  db - pointer pointing to struct DB
  handle - pointer pointing to the handle (book,user,record,etc...)
  id - uid
  model - see enum Model

  Return value:
  DBErrno
 */
int GetById(DB* db, void* handle, unsigned int id, Model model);

/*
 Filter

 Get data by one or more arguments.

 Parameters:

 db - pointer pointing to struct DB
 list_handle - pointer pointing to the list_handle
 pointer(book,user,record,etc...) model - see enum Model

 queries

 Formatting your queries like this:
 name1=value1&name2=value2&name3=value3...
 Ex 1.
 Assume we want to get users that are male:
 "gender=male"

 Ex 2.
 Assume we want to get books that are published by "Cambridge" on March 17,
 2013, and have one of the keyword "Math":
 "press=Cambridge&keyword=Math&published_date=2013-03-17"

 Ex 3.
 Assume we want to filter books by two keywords, "Java" and "Javacript"
 "keyword=Java&keyword=JavaScript"
 Notice that in the above example, the filter is case-sensitive.

 You can also add a ;(semicolon) in front of the = for case-insensitive search.

 Ex 4.
 Assume we want to get books that are published by O Reilly and related to C(so
 you might get books about C/C++/C#/Objective-C!) "press=O Reilly&keyword;=C"

 Return value:
 DBErrno
*/
int Filter(DB* db, void* list_handle, String queries, Model model);

/*
  GetDBSize
  It's used to get the number of rows(elements) in database.
  
  Parameter:
  db - pointer pointing to struct DB
  model - see enum Model
  size - the pointer pointing to the memory to store the size of DB

  Return value:
  DBErrno
*/

int GetDBSize(DB* db, Model model, unsigned int *size);

/*
  GetNextPK
  It's used to get The smallest prime key that isn't mapped to a row in
  database.

  Parameter:
  db - pointer pointing to struct DB
  model - see enum Model

  Return value:
  DBErrno
 */

unsigned int GetNextPK(DB* db, Model model, unsigned int *pk);

// Update
/*
  Update*
  It's used to perform update operation on a row in database.

  Parameter:
  db - pointer pointing to struct DB
  handle - pointer pointing to the handle (book,user,record,etc...)

  id  - uid
  model - see enum Model
  pk - the pointer pointing to the memory to store the primary key

  Return value:
  DBErrno
 */

int Update(DB* db, void* handle, unsigned int id, Model model);

// Delete
/*
  Delete*
  It's used to perform delete operation on a row in database.

  Parameter:
  db - pointer pointing to struct DB
  id - uid
  model - see enum Model

  Return value:
  DBErrno
*/

int Delete(DB* db, unsigned int id, Model model);
#endif  // MODEL_H_
