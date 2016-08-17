#ifndef _DB_MANAGER_PGSQL_
#define _DB_MANAGER_PGSQL_

#ifdef  WIN32
#include <windows.h>
#include <winsock.h>
#endif

#include "libpq-fe.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
/*
	common constants
*/

#define QUERY_BUF_SIZE	200

#define UNIQUE_ID_MAX_LEN 30
#define CALLER_ID_MAX_LEN 15
#define EXTEN_MAX_LEN 15

#define DB_BEFORE_RECONNECT_SLEEP_SEC 2

typedef struct _ASTERISK_DATA{
	char unique_id[UNIQUE_ID_MAX_LEN];
	char linked_id[UNIQUE_ID_MAX_LEN];
	char caller_id_num[CALLER_ID_MAX_LEN];
	char exten[EXTEN_MAX_LEN];
	char call_type[5];
}
ASTERISK_DATA, *PASTERISK_DATA;

/*
	functions
*/
int query(char *qstring);
int connect_to_db(char *db_server,
				char *db_port,
				char *db_user,
				char *db_password,
				char *db_database);
int disconnect_from_db();

void insert_call(PASTERISK_DATA AstData);
void update_call_start(PASTERISK_DATA AstData);

void update_call_start_new_api(PASTERISK_DATA AstData);

void update_call_end(PASTERISK_DATA AstData);
PGconn *db_connection;

extern char *db_server;
extern char *db_user;
extern char *db_password;
extern char *db_database;
extern char *db_port;

extern FILE *fdlog;
		   
#endif
