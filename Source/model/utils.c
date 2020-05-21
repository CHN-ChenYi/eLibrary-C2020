#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include"basictype.h"
#include"utils.h"

// Copy
int SaveStrCpy(char* t, const char* s){
	if(t != NULL && s != NULL) {  strcpy(t,s); return DB_SUCCESS; }
	else return DB_FAIL_ON_FETCHING;
}
int BookCopy(Book* destination, Book* source) { 
	int err = DB_FAIL_ON_FETCHING;
	Book* t = destination, *s = source;
	if( t == NULL || s == NULL) return err; // avoid null ptr.
	t->uid = s->uid;
	err = SaveStrCpy(t->id, s->id); if(err != DB_SUCCESS) return err;
	err = SaveStrCpy(t->title, s->title); if(err != DB_SUCCESS) return err;
	int i;
	for (i = 0; i < 3; i++) {
		err = SaveStrCpy(t->authors[i], s->authors[i]); if(err != DB_SUCCESS) return err;
	}
	err = SaveStrCpy(t->category, s->category); if(err != DB_SUCCESS) return err;
	err = SaveStrCpy(t->press, s->press); if(err != DB_SUCCESS) return err;
	err = SaveStrCpy(t->publication_date, s->publication_date); if(err != DB_SUCCESS) return err;
	for (i = 0; i < 5; i++) {
		err = SaveStrCpy(t->keywords[i], s->keywords[i]); if(err != DB_SUCCESS) return err;
	}
	t->number_on_the_shelf = s->number_on_the_shelf;
	t->available_borrowed_days = s->available_borrowed_days;
	return DB_SUCCESS;
}
int UserCopy(User* destination, User* source) {
	int err = DB_FAIL_ON_FETCHING;
	User* t = destination, *s = source;
	if( t == NULL || s == NULL) return err; // avoid null ptr.
	t->uid = s->uid;
	err = SaveStrCpy(t->id, s->id); if(err != DB_SUCCESS) return err;
	err = SaveStrCpy(t->salt, s->salt); if(err != DB_SUCCESS) return err;
	err = SaveStrCpy(t->name, s->name); if(err != DB_SUCCESS) return err;
	int i;
	for (i = 0; i < 8; i++) t->password[i] = s->password[i];
	t->gender = s->gender;
	err = SaveStrCpy(t->department, s->department); if(err != DB_SUCCESS) return err;
	t->whoami = s->whoami;
	t->verified = s->verified;
	return DB_SUCCESS;
}
int RecordCopy(BorrowRecord* destination, BorrowRecord* source) {
	int err = DB_FAIL_ON_FETCHING;
	BorrowRecord* t = destination, *s = source;
	if( t == NULL || s == NULL) return err; // avoid null ptr.
	t->uid = s->uid;
	t->book_uid = s->book_uid;
	t->user_uid = s->user_uid;
	err = SaveStrCpy(t->book_name, s->book_name); if(err != DB_SUCCESS) return err;
	err = SaveStrCpy(t->user_name, s->user_name); if(err != DB_SUCCESS) return err;
	err = SaveStrCpy(t->borrowed_date, s->borrowed_date); if(err != DB_SUCCESS) return err;
	t->book_status = s->book_status;
	err = SaveStrCpy(t->returned_date, s->returned_date); if(err != DB_SUCCESS) return err;
	return DB_SUCCESS;
}


//Filter
int Cmp(const char* str1, const char* str2, int insensitive, int equal) {
	if (insensitive) return strstr(str1, str2) != NULL ? 1 : 0;
	else if (equal) return strcmp(str1, str2) == 0 ? 1 : 0;
	else return strcmp(str1, str2) == 0 ? 0 : 1;
}

int BookFilter(Book* p_b, String queries) {
	if (strlen(queries) == 0) return 1; // empty queries, always return true.
	char* property, *para; int insensitive = 0, equal = 1, flag = 1;
	char q[1000];
	strcpy(q, queries);
	property = strtok(q, "=");
	while (1) {	
		if (property == NULL) break;
		para = strtok(NULL, "&");
		if (*(property + strlen(property) - 1) == ';') insensitive = 1;
		if (*(property + strlen(property) - 1) == '!') equal = 0;
		if (*(property) == 'u' && *(property + 1) == 'i') {
			char uid_str[50];
			sprintf(uid_str, "%u", p_b->uid);
			flag = Cmp(uid_str, para, insensitive, equal);
		}
		else if (*(property) == 'i' && *(property + 1) == 'd') {
			flag = Cmp(p_b->id, para, insensitive, equal);
		}
		else if (*(property) == 't' && *(property + 1) == 'i') {
			flag = Cmp(p_b->title, para, insensitive, equal);
		}
		else if (*(property) == 'a' && *(property + 1) == 'u') {
			int i;
			for (i = 0; i < 3; i++) {
				flag = Cmp(p_b->authors[i], para, insensitive, equal);
				if (flag) break;
			}
		}
		else if (*(property) == 'c' && *(property + 1) == 'a') {
			flag = Cmp(p_b->category, para, insensitive, equal);
		}
		else if (*(property) == 'p' && *(property + 1) == 'r') {
			flag = Cmp(p_b->press, para, insensitive, equal);
		}
		else if (*(property) == 'p' && *(property + 1) == 'u') {
			flag = Cmp(p_b->publication_date, para, insensitive, equal);
		}
		else if (*(property) == 'k' && *(property + 1) == 'e') {
			int i;
			for (i = 0; i < 5; i++) {
				flag = Cmp(p_b->keywords[i], para, insensitive, equal);
				if (flag) break;
			}
		}
		else if (*(property) == 'n' && *(property + 1) == 'u') {
			char nots_str[50];
			sprintf(nots_str, "%u", p_b->number_on_the_shelf);
			flag = Cmp(nots_str, para, insensitive, equal);
		}
		else if (*(property) == 'a' && *(property + 1) == 'v') {
			char abd_str[50];
			sprintf(abd_str, "%u", p_b->available_borrowed_days);
			flag = Cmp(abd_str, para, insensitive, equal);
		}
		else flag = 0;
		if (!flag) break;
		property = strtok(NULL, "=");
	}
	return flag;
}
int UserFilter(User* p_u, String queries) {
	if (strlen(queries) == 0) return 1;
	char* property, *para; int insensitive = 0, equal = 1, flag = 1;
	char q[1000];
	strcpy(q, queries);
	property = strtok(q, "=");
	while (1) {
		if (property == NULL) break;
		para = strtok(NULL, "&");
		if (*(property + strlen(property) - 1) == ';') insensitive = 1;
		if (*(property + strlen(property) - 1) == '!') equal = 0;
		if (*(property) == 'u' && *(property + 1) == 'i') {
			char uid_str[50];
			sprintf(uid_str, "%u", p_u->uid);
			flag = Cmp(uid_str, para, insensitive, equal);
		}
		else if (*(property) == 'i') {
			flag = Cmp(p_u->id, para, insensitive, equal);
		}
		else if (*(property) == 's' && *(property + 1) == 'a') {
			flag = Cmp(p_u->salt, para, insensitive, equal);
		}
		else if (*(property) == 'n' && *(property + 1) == 'a') {
			flag = Cmp(p_u->name, para, insensitive, equal);
		}
		else if (*(property) == 'p' && *(property + 1) == 'a') {
			int i; char str[50];
			for (i = 0; i < 8; i++) {
				sprintf(str, "%u", p_u->password[i]);
				flag = Cmp(str, para, insensitive, equal);
				if (flag) break;
			}
		}
		else if (*(property) == 'd' && *(property + 1) == 'e') {
			flag = Cmp(p_u->department, para, insensitive, equal);
		}
		else if (*(property) == 'w' && *(property + 1) == 'h') {
			char str[50];
			sprintf(str, "%d", p_u->whoami);
			flag = Cmp(str, para, insensitive, equal);
		}
		else if (*(property) == 'v' && *(property + 1) == 'e') {
			char str[50];
			sprintf(str, "%d", p_u->verified);
			flag = Cmp(str, para, insensitive, equal);
		}
		else flag = 0;
		if (!flag) break;
		property = strtok(NULL, "=");
	}
	return flag;
}
int RecordFilter(BorrowRecord* p_r, String queries) {
	if (strlen(queries) == 0) return 1;
	char* property, *para; int insensitive = 0, equal = 1, flag = 1;
	char q[1000];
	strcpy(q, queries);
	property = strtok(q, "=");
	while (1) {
		if (property == NULL) break;
		para = strtok(NULL, "&");
		if (*(property + strlen(property) - 1) == ';') insensitive = 1;
		if (*(property + strlen(property) - 1) == '!') equal = 0;
		if (*(property) == 'u' && *(property + 1) == 'i') {
			char str[50];
			sprintf(str, "%u", p_r->uid);
			flag = Cmp(str, para, insensitive, equal);
		}
		else if (*(property) == 'b' && *(property + 5) == 'u') {
			char str[50];
			sprintf(str, "%u", p_r->book_uid);
			flag = Cmp(str, para, insensitive, equal);
		}
		else if (*(property) == 'u' && *(property + 5) == 'u') {
			char str[50];
			sprintf(str, "%u", p_r->user_uid);
			flag = Cmp(str, para, insensitive, equal);
		}
		else if (*(property) == 'b' && *(property + 1) == 'n') {
			flag = Cmp(p_r->book_name, para, insensitive, equal);
		}
		else if (*(property) == 'u' && *(property + 5) == 'n') {
			flag = Cmp(p_r->user_name, para, insensitive, equal);
		}
		else if (*(property) == 'b' && *(property + 9) == 'd') {
			flag = Cmp(p_r->borrowed_date, para, insensitive, equal);
		}
		else if (*(property) == 'b' && *(property + 5) == 's') {
			char str[50];
			sprintf(str, "%d", p_r->book_status);
			flag = Cmp(str, para, insensitive, equal);
		}
		else if (*(property) == 'r' && *(property + 9) == 'd') {
			flag = Cmp(p_r->returned_date, para, insensitive, equal);
		}
		else flag = 0;
		if (!flag) break;
		property = strtok(NULL, "=");
	}
	return flag;
}

int Slice(const char* str, char* slice, int* pos){
  char* ch; int i = 0;
  if(slice == NULL) return DB_FAIL_ON_INIT;
  int str_n = strlen(str);
  memset(slice, '\0', sizeof(char) * strlen(slice));
  for(ch = str + (*pos); *(ch+i) != ';' && i < str_n; i++){
    if(*(ch+i) == '\n' || *(ch+i) == '\0') return DB_FAIL_ON_INIT;
    *(slice+i) = *(ch+i);
  }
  *pos += (i+1);
  return DB_SUCCESS;
}

// StringToModel
int StringToBook(Book* p_b, String str) {
	char slice[300] = "";
	int ok; int pos = 0;
	ok = Slice(str, slice, &pos); // uid
	if(ok != DB_SUCCESS) return ok;
	p_b->uid = (unsigned) strtoll(slice, NULL, 10);
	ok = Slice(str, slice, &pos); // id
	if(ok != DB_SUCCESS) return ok;
	SaveStrCpy(p_b->id, slice);
	ok = Slice(str, slice, &pos); // title
	if(ok != DB_SUCCESS) return ok;
	SaveStrCpy(p_b->title, slice);
	int i; // authors
	for (i = 0; i < 3; i++) { 
		ok = Slice(str, slice, &pos); 
		if(ok != DB_SUCCESS) return ok;
		SaveStrCpy(p_b->authors[i], slice);
	}
	ok = Slice(str, slice, &pos); // category
	if(ok != DB_SUCCESS) return ok;
	SaveStrCpy(p_b->category, slice);
	ok = Slice(str, slice, &pos); // press
	if(ok != DB_SUCCESS) return ok;
	SaveStrCpy(p_b->press, slice);
	ok = Slice(str, slice, &pos); // publication_date
	if(ok != DB_SUCCESS) return ok;
	SaveStrCpy(p_b->publication_date, slice);
	for (i = 0; i < 5; i++) { // keywords
		ok = Slice(str, slice, &pos);
		if(ok != DB_SUCCESS) return ok;
		SaveStrCpy(p_b->keywords[i], slice); 
	}
	ok = Slice(str, slice, &pos); // number on the shelf
	if(ok != DB_SUCCESS) return ok;
	p_b->number_on_the_shelf = (unsigned) strtoll(slice, NULL, 10);
	ok = Slice(str, slice, &pos); // available borrowed days
	if(ok != DB_SUCCESS) return ok;
	p_b->available_borrowed_days = (unsigned) strtoll(slice, NULL, 10);
	return DB_SUCCESS;
}
int StringToUser(User* p_u, String str) {
	char slice[300] = "";
	int ok; int pos = 0;
	ok = Slice(str, slice, &pos); // uid
	if(ok != DB_SUCCESS) return ok;
	p_u->uid = (unsigned) strtoll(slice, NULL, 10);
	ok = Slice(str, slice, &pos); // id
	if(ok != DB_SUCCESS) return ok;
	SaveStrCpy(p_u->id, slice);
	ok = Slice(str, slice, &pos); // name
	if(ok != DB_SUCCESS) return ok;
	SaveStrCpy(p_u->name, slice);
	ok = Slice(str, slice, &pos); // salt
	if(ok != DB_SUCCESS) return ok;
	SaveStrCpy(p_u->salt, slice);
	int i;  // password
	for (i = 0; i < 8; i++) { 
		ok = Slice(str, slice, &pos);
		if(ok != DB_SUCCESS) return ok;
		p_u->password[i] = (unsigned) strtoll(slice, NULL, 10); 
	}
	ok = Slice(str, slice, &pos); // gender
	if(ok != DB_SUCCESS) return ok;
	p_u->gender = (unsigned) strtoll(slice, NULL, 10);
	ok = Slice(str, slice, &pos); // department
	if(ok != DB_SUCCESS) return ok;
	SaveStrCpy(p_u->department, slice);
	ok = Slice(str, slice, &pos); // whoami
	if(ok != DB_SUCCESS) return ok;
	p_u->whoami = (unsigned) strtoll(slice, NULL, 10);
	ok = Slice(str, slice, &pos); // verified
	if(ok != DB_SUCCESS) return ok;
	p_u->verified = (unsigned) strtoll(slice, NULL, 10);
	return DB_SUCCESS;
}
int StringToRecord(BorrowRecord* p_r, String str) {
	char slice[300] = "";
	int ok; int pos = 0;
	ok = Slice(str, slice, &pos); // uid
	if(ok != DB_SUCCESS) return ok;
	p_r->uid = (unsigned) strtoll(slice, NULL, 10);
	ok = Slice(str, slice, &pos); // book_uid
	if(ok != DB_SUCCESS) return ok;
	p_r->book_uid = (unsigned) strtoll(slice, NULL, 10);
	ok = Slice(str, slice, &pos); // user_uid
	if(ok != DB_SUCCESS) return ok;
	p_r->user_uid = (unsigned) strtoll(slice, NULL, 10);
	ok = Slice(str, slice, &pos); // book_name
	if(ok != DB_SUCCESS) return ok;
	SaveStrCpy(p_r->book_name, slice);
	ok = Slice(str, slice, &pos); // user_name
	if(ok != DB_SUCCESS) return ok;
	SaveStrCpy(p_r->user_name, slice);
	ok = Slice(str, slice, &pos); // borrowed_date
	if(ok != DB_SUCCESS) return ok;
	SaveStrCpy(p_r->borrowed_date, slice);
	ok = Slice(str, slice, &pos); // book_status
	if(ok != DB_SUCCESS) return ok;
	p_r->book_status = (unsigned) strtoll(slice, NULL, 10);
	ok = Slice(str, slice, &pos); // returned_date
	if(ok != DB_SUCCESS) return ok;
	SaveStrCpy(p_r->returned_date, slice);
	return DB_SUCCESS;
}


int StringToModel(void** handle, Model model, String str) {
	if (model == BOOK) {
		Book* p_b = (Book*)malloc(sizeof(Book));
		StringToBook(p_b, str);
		*handle = (void*)p_b;
	}
	else if (model == USER) {
		User* p_u = (User*)malloc(sizeof(User));
		StringToUser(p_u, str);
		*handle = (void*)p_u;
	}
	else if (model == BORROWRECORD) {
		BorrowRecord* p_br = (BorrowRecord*)malloc(sizeof(BorrowRecord));
		StringToRecord(p_br, str);
		*handle = (void*)p_br;
	}
	return DB_SUCCESS;
}


// ModelToString

#define process(ptr, format, variable)\
  sprintf(p_str_2, format";", ptr->variable); strcat(p_str, (const) p_str_2); 
int ModelToString(void* handle, Model model, char* p_str) {
	char p_str_2[100] = "";
	if (model == BOOK){
		Book* p_b = (Book*)handle;
		process(p_b, "%u", uid);
		process(p_b, "%s", id);
		process(p_b, "%s", title);
		int i;
		for (i = 0; i < 3; i++) { process(p_b, "%s", authors[i]); }
		process(p_b, "%s", category);
		process(p_b, "%s", press);
		process(p_b, "%s", publicatoin_date);
		for (i = 0; i < 5; i++) { process(p_b, "%s", keywords[i]); }
		process(p_b, "%u", number_on_the_shelf);
		process(p_b, "%u", available_borrowed_days);
		strcat(p_str, "\n");
	}
	else if (model == USER) {
		User* p_u = (User*)handle;
		process(p_u, "%u", uid);
		process(p_u, "%s", id);
		process(p_u, "%s", name);
		process(p_u, "%s", salt);
		for (int i = 0; i < 8; i++) {
			process(p_u, "%u", password[i]);
		}
		process(p_u, "%u", gender);
		process(p_u, "%s", department);
		process(p_u, "%u", whoami);
		process(p_u, "%u", verified);
		strcat(p_str, "\n");
	}
	else if (model == BORROWRECORD) {
		BorrowRecord* p_r = (BorrowRecord*)handle;
		process(p_r, "%u", uid);
		process(p_r, "%u", book_uid);
		process(p_r, "%u", user_uid);
		process(p_r, "%s", book_name);
		process(p_r, "%s", user_name);
		process(p_r, "%s", borrowed_date);
		process(p_r, "%u", book_status);
		process(p_r, "%s", returned_date);
		strcat(p_str, "\n");
	}
	return 0;
}
#undef process
