#include"model.h"
#include"basictype.h"
#include"list.h"
#include<stdio.h>

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

static FILE* dbfile;
static List* data;
int OpenDBConnection(DB* db) { 
    dbfile = fopen(db->filename, "a+");
    if (dbfile == NULL){
      return DB_NOT_OPEN;
    }
    
    return DB_SUCCESS; 
}

int CloseDBConnection(DB* db) {
  int ok = fclose(dbfile);
  if (ok != 0){
    return DB_FAIL;
  }
  return DB_SUCCESS;
}

int Create(DB* db, void* handle, Model model) {
  String* p_str;
  int ok;
  ok = ModelToString(handle, model, p_str);
  if (ok != 0){
    return DB_FAIL_ON_WRITING;
  }
  ok = fprintf(dbfile, "%s\n", *p_str);
  if (ok < 0){
    return DB_FAIL_ON_WRITING;
  }
  return DB_SUCCESS;
}

int GetById(DB* db, void* handle, int id, Model model) { return 0; }

int Filter(DB* db, void* list_handle, String queries, Model model) {
  return DB_SUCCESS;
}

int GetNextPK(DB* db, Model model) { return 0; }

int Update(DB* db, void* handle, int id, Model model) { return 0; }

int Delete(DB* db, int id, Model model) { return 0; }
