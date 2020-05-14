#include"model.h"
#include"basictype.h"
#include"list.h"
#include<stdio.h>
#include<string.h>
#define N_MODEL 3
static FILE* databases[N_MODEL];
static List* alldata[N_MODEL];
static int pk[N_MODEL];

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

int DBOpen(DB *db, FILE** database){
  *database = fopen(db->filename, "a+");
  if(*database == NULL) return DB_NOT_OPEN;
  return DB_SUCCESS;
}
int DBClose(DB* db, FILE** database){
  
}
int DBInit(FILE* database, List** data, Model model){
  int ok;
  String str;
  *data = NewList();
  ListNode* current = (*data)->dummy_head;
  if(fgets(str, 1000, database) == NULL){
    return DB_ENTRY_EMPTY;
  }
  ok = sscanf(str, "%d", &pk[model]);
  if(ok != 1) return DB_FAIL_ON_INIT;
  while(fgets(str, 1000, database) != NULL){
    void *handle;
    ok = StringToModel(&handle, model, str);
    if(ok != 0) return DB_FAIL_ON_INIT;
    current = InsertList(*data, current, handle); 
  }
  return DB_SUCCESS;
}
int OpenDBConnection(DB* db, Model model) { 
  int ok; FILE** database; List** data;
  if(!DBExists(model)) return DB_NOT_FOUND;
  database = &databases[model];
  data = &alldata[model];
  ok = DBOpen(db, database);
  if(ok != DB_SUCCESS) return DB_NOT_OPEN; 
  return DBInit(&database, data, model);
}

int CloseDBConnection(DB* db, Model model) {
  int ok; FILE** database; List** data;
}

int Find(void** target, int id, Model model){
  if(model == BOOK){
    ListNode* cur = alldata[model]->dummy_head;
    while(cur != alldata[model]->dummy_tail){
      if(id == ((Book *)cur->value)->uid){
        *target = cur;
        break;
      }
      cur = cur->nxt;
    }
    if(cur == alldata[model]->dummy_tail) return DB_NOT_EXISTS;
    else return DB_SUCCESS;
  }
  else if(model == USER){
    ListNode* cur = alldata[model]->dummy_head;
    while(cur != alldata[model]->dummy_tail){
      if(id == ((User *)cur->value)->uid){
        *target = cur;
        break;
      }
      cur = cur->nxt;
    }
    if(cur == alldata[model]->dummy_tail) return DB_NOT_EXISTS;
    else return DB_SUCCESS;
  }
  else if(model == BORROWRECORD){
    ListNode* cur = alldata[model]->dummy_head;
    while(cur != alldata[model]->dummy_tail){
      if(id == ((BorrowRecord *)cur->value)->uid){
        *target = cur;
        break;
      }
      cur = cur->nxt;
    }
    if(cur == alldata[model]->dummy_tail) return DB_NOT_EXISTS;
    else return DB_SUCCESS;
  }
  else return DB_NOT_EXISTS;
}

int Create(DB* db, void* handle, Model model) {
  String* p_str;
  int ok;
  if(!DBExists(model)) return DB_NOT_FOUND;
  ok = ModelToString(handle, model, p_str);
  if (ok != 0){
    return DB_FAIL_ON_WRITING;
  }
  ok = fprintf(databases[model], "%s\n", *p_str);
  if (p_str != NULL) free(p_str);
  if (ok < 0){ return DB_FAIL_ON_WRITING; }
  else return DB_SUCCESS;
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

int GetById(DB* db, void* handle, int id, Model model) { 
  int ok;
  if(!DBExists(model)) return DB_NOT_FOUND;
  ListNode* cur = alldata[model]->dummy_head;
  void* cur;
  ok = Find(&cur, id, model);
  if(ok == 0){
    if(model == BOOK) return BookCopy((Book*) handle, (Book*) cur);
    else if(model == USER) return UserCopy((User*) handle, (User*) cur);
    else if(model == USER) return RecordCopy((BorrowRecord*) handle, (BorrowRecord*) cur);
  }
  return DB_NOT_EXISTS;  
}

int Filter(DB* db, void* list_handle, String queries, Model model) {
  return DB_SUCCESS;
}

int GetNextPK(DB* db, Model model) { 
  if(!DBExists(model)) return DB_NOT_FOUND;
  return pk[model]+1; 
}

int Update(DB* db, void* handle, int id, Model model){ 
  if(!DBExists(model)) return DB_NOT_FOUND;
  void* target;
  int ok, uid;
  ok = Find(&target, id, model);
  if(ok != DB_SUCCESS) return DB_NOT_EXISTS;
  if(model == BOOK) uid = ((Book*) target)->uid;
  else if(model == USER) uid = ((User*) target)->uid;
  else if(model == BORROWRECORD) uid = ((BorrowRecord*) target)->uid;
  ok = Delete(db, uid, model);
  if(ok != DB_SUCCESS) return DB_FAIL_ON_UPDATE;
  ok = Create(db, handle, model);
  if(ok != DB_SUCCESS) return DB_FAIL_ON_UPDATE;
  return ok;
}

int Delete(DB* db, int id, Model model) { return 0; }
