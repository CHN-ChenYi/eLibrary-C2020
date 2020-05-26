#include"model.h"
#include"basictype.h"
#include"list.h"
#include"utils.h"
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#define N_MODEL 3
static DB DBs[N_MODEL];

inline int DBExists(Model model) {
	return (model >= 0 && model <= N_MODEL);
}

int DBOpen(const char* filename, Model model) {
	FILE* database;
	database = fopen(filename, "r+");
	if (database == NULL) {
		database = fopen(filename, "w+");
	}
	DBs[model].filename = (char *)malloc(sizeof(char) * 500);
	strcpy(DBs[model].filename, filename);
	DBs[model].database = database;
	return DB_SUCCESS;
}
int DBClose(Model model) {
	int ok;
	ok = fclose(DBs[model].database);
	if(DBs[model].filename != NULL) free(DBs[model].filename);
	DBs[model].filename = NULL;
	if (ok != 0) return DB_NOT_CLOSE;
	else return DB_SUCCESS;
}
int DBInit(Model model) {
	int ok;
	char str[1000] = "";
	FILE** database = &DBs[model].database;
	List** data = &DBs[model].data;
	*data = NewList();
	if (fgets(str, 50, *database) == NULL) {
		if (strlen(str) == 0) {
			DBs[model].pk = 0;
			DBs[model].size = 0;
			return DB_SUCCESS;
		}
		return DB_ENTRY_EMPTY;
	}
	ok = sscanf(str, "%d %d", &(DBs[model].pk), &(DBs[model].size));
	if (ok != 2) return DB_FAIL_ON_INIT;
	while (fgets(str, 1000, *database) != NULL) {
		void *handle;
		ok = StringToModel(&handle, model, str);
		if (ok != 0) return DB_FAIL_ON_INIT;
		InsertList(*data, (*data)->dummy_tail, handle);
	}
	return DB_SUCCESS;
}
int DBUninit(Model model) {
	int ok;
	FILE* database = DBs[model].database;
	if (database == NULL) return DB_NOT_FOUND;
	List* data = DBs[model].data;
	ListNode* cur = data->dummy_head;
	ok = fseek(database, 0, SEEK_SET); // reset FILE pointer to its starting point.
	if (ok != 0) {
		ClearList(data, NULL);
		return DB_FAIL_ON_UNINIT;
	}
	char firstline[150];
	sprintf(firstline, "%d %d\n", DBs[model].pk, DBs[model].size);
	fwrite(firstline, sizeof(char), sizeof(firstline)-1, database); // avoid \0 output
	//fprintf(database, "%d %d\n", DBs[model].pk, DBs[model].size);
	while (1) {
		cur = cur->nxt;
		if (cur == data->dummy_tail) break;
		char str[3000] = "";
		ok = ModelToString(cur->value, model, str);
		if (ok != 0) {
			ClearList(data, NULL);
			return DB_FAIL_ON_UNINIT;
		}
		fwrite (str , sizeof(char), sizeof(str)-1, database); // avoid \0 output
		//fprintf(database, "%s", str);
	}
	ClearList(data, NULL);
	return DB_SUCCESS;
}
int OpenDBConnection(const char* filename, Model model) {
	int ok;
	if (!DBExists(model)) return DB_NOT_FOUND;
	ok = DBOpen(filename, model);
	if (ok != DB_SUCCESS) return DB_NOT_OPEN;
	ok = DBInit(model);
	return ok;
}
int CloseDBConnection(Model model) {
	int ok;
	if (!DBExists(model)) return DB_NOT_FOUND;
	ok = DBUninit(model);
	if (ok != DB_SUCCESS) return DB_NOT_OPEN;
	ok = DBClose(model);
	return ok;
}

int Find(ListNode** target, unsigned int id, Model model) {
	if (model == BOOK) {
		ListNode* cur = DBs[model].data->dummy_head;
		while (1) {
			cur = cur->nxt;
			if (cur == DBs[model].data->dummy_tail) break;
			if (id == ((Book *)cur->value)->uid) {
				*target = cur;
				break;
			}
		}
		if (cur == DBs[model].data->dummy_tail) return DB_NOT_EXISTS;
		else return DB_SUCCESS;
	}
	else if (model == USER) {
		ListNode* cur = DBs[model].data->dummy_head;
		while (1) {
			cur = cur->nxt;
			if (cur == DBs[model].data->dummy_tail) break;
			if (id == ((User *)cur->value)->uid) {
				*target = cur;
				break;
			}
		}
		if (cur == DBs[model].data->dummy_tail) return DB_NOT_EXISTS;
		else return DB_SUCCESS;
	}
	else if (model == BORROWRECORD) {
		ListNode* cur = DBs[model].data->dummy_head;
		while (1) {
			cur = cur->nxt;
			if (cur == DBs[model].data->dummy_tail) break;
			if (id == ((BorrowRecord *)cur->value)->uid) {
				*target = cur;
				break;
			}
		}
		if (cur == DBs[model].data->dummy_tail) return DB_NOT_EXISTS;
		else return DB_SUCCESS;
	}
	else return DB_NOT_EXISTS;
}

int Create(void* handle, Model model) {
	if (!DBExists(model)) return DB_NOT_FOUND;
	void* target = NULL;
	int ok;
	if (model == BOOK) {
		target = malloc(sizeof(Book));
		ok = BookCopy((Book*)target, (Book*)handle);
		if (ok != DB_SUCCESS) return DB_FAIL_ON_CREATE;
	}
	else if (model == USER) {
		target = malloc(sizeof(User));
		ok = UserCopy((User*)target, (User*)handle);
		if (ok != DB_SUCCESS) return DB_FAIL_ON_CREATE;
	}
	else if (model == BORROWRECORD) {
		target = malloc(sizeof(BorrowRecord));
		ok = RecordCopy((BorrowRecord*)target, (BorrowRecord*)handle);
		if (ok != DB_SUCCESS) return DB_FAIL_ON_CREATE;
	}
	if (target == NULL) return DB_FAIL_ON_CREATE;
	if (InsertList(DBs[model].data, DBs[model].data->dummy_tail, target) == DBs[model].data->dummy_tail) {
		return DB_FAIL_ON_CREATE;
	}
	DBs[model].pk++;
	DBs[model].size++;
	return DB_SUCCESS;
}

int GetById(void* handle, unsigned int id, Model model) {
	int ok;
	if (!DBExists(model)) return DB_NOT_FOUND;
	ListNode* cur = NULL;
	ok = Find(&cur, id, model);
	if (ok == 0) {
		if (model == BOOK) return BookCopy((Book*)handle, (Book*)cur->value);
		else if (model == USER) return UserCopy((User*)handle, (User*)cur->value);
		else if (model == USER) return RecordCopy((BorrowRecord*)handle, (BorrowRecord*)cur->value);
	}
	return DB_NOT_EXISTS;
}

int Filter(List* list_handle, String queries, Model model) {
	List* data;
	data = DBs[model].data;
	int ok;
	if ( !DBExists(model) || data == NULL) return DB_NOT_FOUND;
	ListNode* cur = data->dummy_head;
	List* handle = list_handle;
	if (model == BOOK) {
		while (1) {
			cur = cur->nxt;
			if (cur == data->dummy_tail) break;
			if (BookFilter(cur->value, queries)) {
				Book* p_b = (Book*)malloc(sizeof(Book));
				ok = BookCopy(p_b, (Book*)cur->value);
				if(ok != DB_SUCCESS) return ok;
				InsertList(handle, handle->dummy_tail, p_b);
			}
		}
	}
	else if (model == USER) {
		while (1) {
			cur = cur->nxt;
			if (cur == data->dummy_tail) break;
			if (UserFilter(cur->value, queries)) {
				User* p_u = (User*)malloc(sizeof(User));
				ok = UserCopy(p_u, (User*)cur->value);
				if(ok != DB_SUCCESS) return ok;
				InsertList(handle, handle->dummy_tail, p_u);
			}
		}
	}
	else if (model == BORROWRECORD) {
		while (1) {
			cur = cur->nxt;
			if (cur == data->dummy_tail) break;
			if (RecordFilter(cur->value, queries)) {
				BorrowRecord* p_r = (BorrowRecord*)malloc(sizeof(BorrowRecord));
				ok = RecordCopy(p_r, (BorrowRecord*)cur->value);
				if(ok != DB_SUCCESS) return ok;
				InsertList(handle, handle->dummy_tail, p_r);
			}
		}
	}
	return DB_SUCCESS;
}

int GetDBSize(Model model, unsigned int *size) {
	if (!DBExists(model)) return DB_NOT_FOUND;
	*size = DBs[model].size;
	return DB_SUCCESS;
}

int GetNextPK(Model model, unsigned int *pk) {
	if (!DBExists(model)) return DB_NOT_FOUND;
	*pk = DBs[model].pk;
	return DB_SUCCESS;
}

int Update(void* handle, unsigned int id, Model model) {
	if (!DBExists(model)) return DB_NOT_FOUND;
	ListNode* target;
	int ok, uid = id;
	ok = Find(&target, id, model);
	if (ok != DB_SUCCESS) return DB_NOT_EXISTS;
	if (model == BOOK) {
		BookCopy((Book*) target->value, (Book*)handle);
		((Book*)target->value)->uid = uid;
	} 
	else if (model == USER) {
		UserCopy((User*)target->value, (User*)handle);
		((User*)target->value)->uid = uid;
	}
	else if (model == BORROWRECORD) {
		RecordCopy((BorrowRecord*)target->value, (BorrowRecord*)handle);
		((BorrowRecord*)target->value)->uid = uid;
	}
	return ok;
}

int Delete(unsigned int id, Model model) {
	ListNode* target;
	List* data = DBs[model].data;
	int ok;
	ok = Find(&target, id, model);
	if (ok != DB_SUCCESS) return DB_FAIL_ON_DELETE;
	EraseList(data, target, NULL);
	DBs[model].size--;
	return DB_SUCCESS;
}
