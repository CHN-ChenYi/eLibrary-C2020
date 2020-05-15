#include"model.h"
#include"basictype.h"
#include"list.h"
#include<stdio.h>
#include<string.h>
#define N_MODEL 3
static DB DBs[N_MODEL];

inline int DBExists(Model model){
  return (model >= 0 && model <= N_MODEL);
}
int StringToModel(void** handle, Model model, String str){

}
int ModelToString(void* handle, Model model, String* p_str){
  switch (model){
    case BOOK:
      /* code */
      break; 
    case USER:

      break;
    case BORROWRECORD:

      break;
    default:
      break;
  }
}

int DBOpen(const char* filename, Model model){
  FILE* database;
  database = fopen(filename, "a+");
  if(database == NULL) return DB_NOT_OPEN;
  DBs[model].filename = filename;
  DBs[model].database = database;
  return DB_SUCCESS;
}
int DBClose(Model model){
  int ok;
  ok = fclose(DBs[model].filename);
  if(ok != 0) return DB_NOT_CLOSE;
  else return DB_SUCCESS;
}
int DBInit(Model model){
  int ok;
  String str;
  FILE** database = &DBs[model].database;
  List** data = &DBs[model].data;
  *data = NewList();
  ListNode* current = (*data)->dummy_head;
  if(fgets(str, 50, database) == NULL){
    return DB_ENTRY_EMPTY;
  }
  ok = sscanf(str, "%d %d", &(DBs[model].pk), &(DBs[model].size));
  if(ok == EOF){
    DBs[model].pk = 0;
    DBs[model].size = 0;
    return DB_SUCCESS;
  }
  else if(ok != 2) return DB_FAIL_ON_INIT;
  while(fgets(str, 1000, database) != NULL){
    void *handle;
    ok = StringToModel(&handle, model, str);
    if(ok != 0) return DB_FAIL_ON_INIT;
    current = InsertList(*data, current, handle); 
  }
  return DB_SUCCESS;
}

int DBUninit(Model model){
  int ok;
  String str;
  FILE* database = DBs[model].database; 
  List* data = DBs[model].data;
  ListNode* cur = data->dummy_head;
  ok = fseek(database, 0, SEEK_SET); // reset FILE pointer to its starting point.
  if(ok != 0){
    ClearList(data, NULL);
    return DB_FAIL_ON_UNINIT;
  }
  fprintf(database, "%d %d", DBs[model].pk, DBs[model].size);
  while(cur = cur->nxt && cur != data->dummy_tail){
    ok = ModelToString(cur->value, model, &str);
    if(ok != 0){
      ClearList(data, NULL);
      return DB_FAIL_ON_UNINIT;
    }
    fprintf(database, "\n%s", str);
  }
  ClearList(data, NULL);
  return DB_SUCCESS;
}

int OpenDBConnection(const char* filename, Model model) { 
  int ok; 
  if(!DBExists(model)) return DB_NOT_FOUND;
  ok = DBOpen(filename, model);
  if(ok != DB_SUCCESS) return DB_NOT_OPEN; 
  ok = DBInit(model);
  return ok;
}

int CloseDBConnection(Model model) {
  int ok;
  if(!DBExists(model)) return DB_NOT_FOUND;
  ok = DBUninit(model);
  if(ok != DB_SUCCESS) return DB_NOT_OPEN; 
  ok = DBClose(model);
  return ok;
}

int Find(ListNode** target, unsigned int id, Model model){
  if(model == BOOK){
    ListNode* cur = DBs[model].data->dummy_head;
    while(cur != DBs[model].data->dummy_tail){
      if(id == ((Book *)cur->value)->uid){
        *target = cur;
        break;
      }
      cur = cur->nxt;
    }
    if(cur == DBs[model].data->dummy_tail) return DB_NOT_EXISTS;
    else return DB_SUCCESS;
  }
  else if(model == USER){
    ListNode* cur = DBs[model].data->dummy_head;
    while(cur != DBs[model].data->dummy_tail){
      if(id == ((User *)cur->value)->uid){
        *target = cur;
        break;
      }
      cur = cur->nxt;
    }
    if(cur == DBs[model].data->dummy_tail) return DB_NOT_EXISTS;
    else return DB_SUCCESS;
  }
  else if(model == BORROWRECORD){
    ListNode* cur = DBs[model].data->dummy_head;
    while(cur != DBs[model].data->dummy_tail){
      if(id == ((BorrowRecord *)cur->value)->uid){
        *target = cur;
        break;
      }
      cur = cur->nxt;
    }
    if(cur == DBs[model].data->dummy_tail) return DB_NOT_EXISTS;
    else return DB_SUCCESS;
  }
  else return DB_NOT_EXISTS;
}

int Create(void* handle, Model model) {
  int ok;
  if(!DBExists(model)) return DB_NOT_FOUND;
  if(InsertList(DBs[model].data, DBs[model].data->dummy_tail, handle) == DBs[model].data->dummy_tail){
    return DB_FAIL_ON_CREATE;
  }
  DBs[model].pk++;
  DBs[model].size++;
  return DB_SUCCESS;
}

int BookCopy(Book* destination, Book* source){
  int err = DB_FAIL_ON_FETCHING;
  destination->uid = source->uid;
  if(strcpy(destination->id, source->id) == NULL) return err;
  if(strcpy(destination->title, source->title) == NULL) return err;
  int i;
  for(i = 0; i < 3; i++){
      if(strcpy(destination->authors[i], source->authors[i]) == NULL) return err;
  }
  if(strcpy(destination->category, source->category) == NULL) return err;
  if(strcpy(destination->press, source->press) == NULL) return err;
  for(i = 0; i < 5; i++){
    if(strcpy(destination->keywords[i], source->keywords[i]) == NULL) return err;
  }
    destination->number_on_the_shelf = source->number_on_the_shelf;
    destination->available_borrowed_days = source->available_borrowed_days;
  return DB_SUCCESS;
}
int UserCopy(User* destination, User* source){
  int err = DB_FAIL_ON_FETCHING;
  destination->uid = source->uid;
  if(strcpy(destination->username, source->username) == NULL) return err;
  if(strcpy(destination->salt, source->salt) == NULL) return err;
  int i;
  for(i = 0; i < 8; i++) destination->password[i] = source->password[i];
  destination->gender = source->gender;
  if(strcpy(destination->department, source->department) == NULL) return err;
  destination->whoami = source->whoami;
  destination->verified = 0;
}
int RecordCopy(BorrowRecord* destination, BorrowRecord* source){
  int err = DB_FAIL_ON_FETCHING;
  destination->uid = source->uid;
  destination->book_uid = source->book_uid;
  destination->user_uid = source->user_uid;
  if(strcpy(destination->book_name, source->book_name) == NULL) return err;
  if(strcpy(destination->user_name, source->user_name) == NULL) return err;
  if(strcpy(destination->borrowed_date, source->borrowed_date) == NULL) return err;
  destination->book_status = source->book_status;
  if(strcpy(destination->returned_date, source->returned_date) == NULL) return err;
  return DB_SUCCESS;
}

int GetById(void* handle, unsigned int id, Model model) { 
  int ok;
  if(!DBExists(model)) return DB_NOT_FOUND;
  ListNode* cur = DBs[model].data->dummy_head;
  void* cur;
  ok = Find(&cur, id, model);
  if(ok == 0){
    if(model == BOOK) return BookCopy((Book*) handle, (Book*) cur);
    else if(model == USER) return UserCopy((User*) handle, (User*) cur);
    else if(model == USER) return RecordCopy((BorrowRecord*) handle, (BorrowRecord*) cur);
  }
  return DB_NOT_EXISTS;  
}

int Filter(void* list_handle, String queries, Model model) {
  return DB_SUCCESS;
}

int GetDBSize(Model model, unsigned int *size){
  if(!DBExists(model)) return DB_NOT_FOUND;
  *size = DBs[model].size + 1;
  return DB_SUCCESS;
}

int GetNextPK(Model model, unsigned int *pk) { 
  if(!DBExists(model)) return DB_NOT_FOUND;
  *pk = DBs[model].pk + 1;
  return DB_SUCCESS;
}

int Update(void* handle, unsigned int id, Model model){ 
  if(!DBExists(model)) return DB_NOT_FOUND;
  void* target;
  int ok, uid;
  ok = Find(&target, id, model);
  if(ok != DB_SUCCESS) return DB_NOT_EXISTS;
  if(model == BOOK) uid = ((Book*) target)->uid;
  else if(model == USER) uid = ((User*) target)->uid;
  else if(model == BORROWRECORD) uid = ((BorrowRecord*) target)->uid;
  ok = Delete(uid, model);
  if(ok != DB_SUCCESS) return DB_FAIL_ON_DELETE;
  ok = Create(handle, model);
  if(ok != DB_SUCCESS) return DB_FAIL_ON_UPDATE;
  return ok;
}

int Delete(unsigned int id, Model model) { 
  ListNode* target;
  List* data = DBs[model].data;
  int ok;
  ok = Find(&target, id, model);
  if(ok != DB_SUCCESS) return DB_FAIL_ON_DELETE;
  EraseList(data, target, NULL);
  DBs[model].size--;
  return DB_SUCCESS; 
}
