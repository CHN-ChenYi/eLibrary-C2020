#include"model.h"
#include<stdio.h>

int OpenDBConnection(DB* db) { return 0; }

int CloseDBConnection(DB* db) { return 0; }

int Create(DB* db, void** handle, Model model) { return 0; }

int GetById(DB* db, void** handle, int id, Model model) { return 0; }

int Filter(DB* db, void** list_handle, String queries, Model model) {
  return 0;
}

int GetNextPK(DB* db, Model model) { return 0; }

int Update(DB* db, void** handle, int id, Model model) { return 0; }

int Delete(DB* db, int id, Model model) { return 0; }
