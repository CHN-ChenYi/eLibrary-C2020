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
	err = SaveStrCpy(t->book_id, s->book_id); if(err != DB_SUCCESS) return err;
	err = SaveStrCpy(t->user_id, s->user_id); if(err != DB_SUCCESS) return err;
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
		else if (*(property) == 'g' && *(property + 1) == 'e') {
			char str[50];
			sprintf(str, "%d", p_u->gender);
			flag = Cmp(str, para, insensitive, equal);
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
		else if (*(property) == 'b' && *(property + 5) == 'i') {
			flag = Cmp(p_r->book_id, para, insensitive, equal);
		}
		else if (*(property) == 'u' && *(property + 5) == 'i') {
			flag = Cmp(p_r->user_id, para, insensitive, equal);
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
