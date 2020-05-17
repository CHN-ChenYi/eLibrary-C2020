#include"model.h"
#include"basictype.h"
#include"list.h"
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#define N_MODEL 3
static DB DBs[N_MODEL];

inline int DBExists(Model model){
  return (model >= 0 && model <= N_MODEL);
}
#define process_to_unsigned(ptr, str, variable) \
pch = strtok(str, ";\n"); \
ptr->variable = (unsigned) strtol(pch, NULL, 10);
#define process_to_str(ptr, str, variable) \
pch = strtok(str, ";\n"); \
strcpy(ptr->variable, pch);

int StringToBook(Book* p_b, String str){
  char *pch;
  process_to_unsigned(p_b, str, uid);
  process_to_str(p_b, NULL, id);
  process_to_str(p_b, NULL, title);
  int i;
  for(i = 0; i < 3; i++) process_to_str(p_b, NULL, authors[i]);
  process_to_str(p_b, NULL, category);
  process_to_str(p_b, NULL, press);
  for(i = 0; i < 5; i++) process_to_str(p_b, NULL, keywords[i]);
  process_to_unsigned(p_b, NULL, number_on_the_shelf);
  process_to_unsigned(p_b, NULL, available_borrowed_days);
  return 0;
}
int StringToUser(User* p_u, String str){
  char* pch;
  process_to_unsigned(p_u, str, uid);
  process_to_str(p_u, NULL, username);
  process_to_str(p_u, NULL, salt);
  int i;
  for(i = 0; i < 8; i++) process_to_unsigned(p_u, NULL, password[i]);
  process_to_unsigned(p_u, NULL, gender);
  process_to_str(p_u, NULL, department);
  process_to_unsigned(p_u, NULL, whoami);
  process_to_unsigned(p_u, NULL, verified);
  return 0;
}
int StringToRecord(BorrowRecord* p_r, String str){
  char* pch;
  process_to_unsigned(p_r, str, uid);
  process_to_unsigned(p_r, NULL, book_uid);
  process_to_unsigned(p_r, NULL, user_uid);
  process_to_str(p_r, NULL, book_name);
  process_to_str(p_r, NULL, user_name);
  process_to_str(p_r, NULL, borrowed_date);
  process_to_unsigned(p_r, NULL, book_status);
  process_to_str(p_r, NULL, returned_date);
  return 0;
}
#undef process_to_unsigned
#undef process_to_str

int StringToModel(void** handle, Model model, String str){
  if(model == BOOK){
    Book* p_b = (Book* ) malloc(sizeof(Book));
    StringToBook(p_b, str);
    *handle = (void* ) p_b;
  }
  else if(model == USER){
    User* p_u = (User* ) malloc(sizeof(User));
    StringToUser(p_u, str);
    *handle = (void* ) p_u;
  }
  else if(model == BORROWRECORD){
    BorrowRecord* p_br = (BorrowRecord* ) malloc(sizeof(BorrowRecord));
    StringToUser(p_br, str);
    *handle = (void* ) p_br;
  }
  return 0;
}

#define process(ptr, format, variable)\
  sprintf(p_str_2, format";", ptr->variable); strcat(*p_str, p_str_2); 
int ModelToString(void* handle, Model model, String* p_str){
  String p_str_2;
  if(model == BOOK){
    Book* p_b = (Book* ) handle;
    process(p_b, "%u", uid);
    process(p_b, "%s", id);
    int i;
    for(i = 0; i < 3; i++) process(p_b, "%s", authors[i]);
    process(p_b, "%s", category);
    process(p_b, "%s", press);
    for(i = 0; i < 5; i++) process(p_b, "%s", keywords[i]);
    process(p_b, "%u", number_on_the_shelf);
    process(p_b, "%u", available_borrowed_days);
    strcat(*p_str, "\n");
  }
  else if(model == USER){
    User* p_u = (User* ) handle;
    process(p_u, "%u", uid);
    process(p_u, "%s", username);
    process(p_u, "%s", salt);
    for(int i = 0; i < 8 ; i++){
      process(p_u, "%u", password[i]);
    }
    process(p_u, "%u", gender);
    process(p_u, "%s", department);
    process(p_u, "%u", whoami);
    process(p_u, "%u", verified);
    strcat(*p_str, "\n");
  }
  else if(model == BORROWRECORD){
    BorrowRecord* p_r = (BorrowRecord* ) handle;
    process(p_r, "%u", uid);
    process(p_r, "%u", book_uid);
    process(p_r, "%u", user_uid);
    process(p_r, "%s", book_name);
    process(p_r, "%s", user_name);
    process(p_r, "%s", borrowed_date);
    process(p_r, "%u", book_status);
    process(p_r, "%s", returned_date);
    strcat(*p_str, "\n");
  }
  return 0;  
}
#undef process

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

int Cmp(const char str1, const char str2, int insensitive, int equal){
  if(insensitive) return strstr(str1, str2) != NULL ? 1 : 0;
  else if(equal) return strcmp(str1, str2) == 0 ? 1 : 0;
  else return strcmp(str1, str2) == 0 ? 0 : 1;
}

int BookFilter(Book* p_b, String queries){
  if(strlen(queries) == 0) return 1;
  char* property, *para; int insensitive = 0, equal = 1, flag = 1;
  property = strtok(queries, "=");
  while(1){
    if(property == NULL) break;
    para = strtok(NULL, "&");
    if(*(property + strlen(property) - 2) == ';') insensitive = 1;
    if(*(property + strlen(property) - 2) == '!') equal = 0;
    if(*(property) == 'u' && *(property+1) == 'i'){
      char uid_str[50];
      sprintf(uid_str, "%u", p_b->uid);
      flag = Cmp(uid_str, para, insensitive, equal);
    }
    else if(*(property) == 'i' && *(property+1) == 'd'){
      flag = Cmp(p_b->id, para, insensitive, equal);
    }
    else if(*(property) == 'a' && *(property+1) == 'u'){
      int i;
      for( i = 0; i < 3; i++){
        flag = Cmp(p_b->authors[i], para, insensitive, equal);
        if(flag) break;
      } 
    }
    else if(*(property) == 'c' && *(property+1) == 'a'){
      flag = Cmp(p_b->category, para, insensitive, equal);
    }
    else if(*(property) == 'p' && *(property+1) == 'r'){
      flag = Cmp(p_b->press, para, insensitive, equal);
    }
    else if(*(property) == 'k' && *(property+1) == 'e'){
      int i;
      for( i = 0; i < 5; i++){
        flag = Cmp(p_b->keywords[i], para, insensitive, equal);
        if(flag) break;
      } 
    } 
    else if(*(property) == 'n' && *(property+1) == 'u'){
      char nots_str[50];
      sprintf(nots_str, "%u", p_b->number_on_the_shelf);
      flag = Cmp(nots_str, para, insensitive, equal);
    }
    else if(*(property) == 'a' && *(property+1) == 'v'){
      char abd_str[50];
      sprintf(abd_str, "%u", p_b->available_borrowed_days);
      flag = Cmp(abd_str, para, insensitive, equal);
    }
    else flag = 0;
    if(!flag) break;
    property = strtok(NULL, "=");
  }
}

int UserFilter(User* p_u, String queries){
  if(strlen(queries) == 0) return 1;
  char* property, *para; int insensitive = 0, equal = 1, flag = 1;
  property = strtok(queries, "=");
  while(1){
    if(property == NULL) break;
    para = strtok(NULL, "&");
    if(*(property + strlen(property) - 2) == ';') insensitive = 1;
    if(*(property + strlen(property) - 2) == '!') equal = 0;
    if(*(property) == 'u' && *(property+1) == 'i'){
      char uid_str[50];
      sprintf(uid_str, "%u", p_u->uid);
      flag = Cmp(uid_str, para, insensitive, equal);
    }
    else if(*(property) == 'u' && *(property+1) == 's'){
      flag = Cmp(p_u->username, para, insensitive, equal);
    }
    else if(*(property) == 's' && *(property+1) == 'a'){
      flag = Cmp(p_u->salt, para, insensitive, equal);
    }
    else if(*(property) == 'p' && *(property+1) == 'a'){
      int i; char str[50];
      for( i = 0; i < 8; i++){
        sprintf(str, "%u", p_u->password[i]);
        flag = Cmp(str, para, insensitive, equal);
        if(flag) break;
      } 
    } 
    else if(*(property) == 'd' && *(property+1) == 'e'){
      flag = Cmp(p_u->department, para, insensitive, equal);
    }
    else if(*(property) == 'w' && *(property+1) == 'h'){
      char str[50];
      sprintf(str, "%d", p_u->whoami);
      flag = Cmp(str, para, insensitive, equal);
    }
    else if(*(property) == 'v' && *(property+1) == 'e'){
      char str[50];
      sprintf(str, "%d", p_u->verified);
      flag = Cmp(str, para, insensitive, equal);
    }
    else flag = 0;
    if(!flag) break;
    property = strtok(NULL, "=");
  }
}

int RecordFilter(BorrowRecord* p_r, String queries){
  if(strlen(queries) == 0) return 1;
  char* property, *para; int insensitive = 0, equal = 1, flag = 1;
  property = strtok(queries, "=");
  while(1){
    if(property == NULL) break;
    para = strtok(NULL, "&");
    if(*(property + strlen(property) - 2) == ';') insensitive = 1;
    if(*(property + strlen(property) - 2) == '!') equal = 0;
    if(*(property) == 'u' && *(property+1) == 'i'){
      char str[50];
      sprintf(str, "%u", p_r->uid);
      flag = Cmp(str, para, insensitive, equal);
    }
    else if(*(property) == 'b' && *(property+5) == 'u'){
      char str[50];
      sprintf(str, "%u", p_r->book_uid);
      flag = Cmp(str, para, insensitive, equal);
    }
    else if(*(property) == 'u' && *(property+5) == 'u'){
      char str[50];
      sprintf(str, "%u", p_r->user_uid);
      flag = Cmp(str, para, insensitive, equal);
    }
    else if(*(property) == 'b' && *(property+1) == 'n'){
      flag = Cmp(p_r->book_name, para, insensitive, equal);
    }
    else if(*(property) == 'u' && *(property+5) == 'n'){
      flag = Cmp(p_r->user_name, para, insensitive, equal);
    } 
    else if(*(property) == 'b' && *(property+9) == 'd'){
      flag = Cmp(p_r->borrowed_date, para, insensitive, equal);
    }
    else if(*(property) == 'b' && *(property+5) == 's'){
      char str[50];
      sprintf(str, "%d", p_r->book_status);
      flag = Cmp(str, para, insensitive, equal);
    }
    else if(*(property) == 'r' && *(property+9) == 'd'){
      flag = Cmp(p_r->returned_date, para, insensitive, equal);
    }
    else flag = 0;
    if(!flag) break;
    property = strtok(NULL, "=");
  }
}

int Filter(void* list_handle, String queries, Model model) {
  List* handle = NewList();
  List* data;
  if(model == BOOK){
    data = DBs[model].data;
    ListNode* cur = data->dummy_head;
    while(cur = cur->nxt && cur != data->dummy_tail){
      if(BookFilter(cur->value, queries)){
        Book* p_b = (Book* ) malloc(sizeof(Book));
        BookCopy(p_b, (Book* )cur->value);
        InsertList(handle, handle->dummy_tail, p_b);
      }
    } 
  }
  else if(model == USER){
    data = DBs[model].data;
    ListNode* cur = data->dummy_head;
    while(cur = cur->nxt && cur != data->dummy_tail){
      if(UserFilter(cur->value, queries)){
        User* p_u = (User* ) malloc(sizeof(User));
        UserCopy(p_u, (User* )cur->value);
        InsertList(handle, handle->dummy_tail, p_u);
      }
    }
  }
  else if(model == BORROWRECORD){
    data = DBs[model].data;
    ListNode* cur = data->dummy_head;
    while(cur = cur->nxt && cur != data->dummy_tail){
      if(RecordFilter(cur->value, queries)){
        BorrowRecord* p_r = (BorrowRecord* ) malloc(sizeof(BorrowRecord));
        BookCopy(p_r, (BorrowRecord* )cur->value);
        InsertList(handle, handle->dummy_tail, p_r);
      }
    }
  }
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
