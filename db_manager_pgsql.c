#include "db_manager_pgsql.h"

/*------------------------------------*/
int db_conn_ok(){
	if (PQstatus(db_connection) == CONNECTION_OK){
		return 1;
	}
	PQreset(db_connection);
	if (PQstatus(db_connection) == CONNECTION_OK){
		return 1;
	}
	return 0;
}

/* Connect to database*/
int connect_to_db(char *db_server,
				char *db_port,
				char *db_user,
				char *db_password,
				char *db_database){   
	if (!db_connection){
		db_connection = PQsetdbLogin(db_server,
				db_port,
				NULL,
				NULL,
				db_database,db_user,db_password);	
	}
	
	time_t t;
	time(&t);				
	while (!db_conn_ok()){
		fprintf(stderr,"%s %s\n",
				ctime(&t),
				PQerrorMessage(db_connection));							
		sleep(DB_BEFORE_RECONNECT_SLEEP_SEC);
	}
	
}

int disconnect_from_db(){   
	if (db_connection){
		PQfinish(db_connection);
	}
	return(1);
}

int query(char *qstring){
	time_t t;
	time(&t);				
	
	#ifdef _DEBUG
	fprintf(stdout,"%s Query=%s\n",
		ctime(&t),
		qstring);
	#endif
	
	if (db_connection){
		PGresult   *res;
		
		while (!db_conn_ok()){
			fprintf(stderr,"%s %s\n",
					ctime(&t),
					PQerrorMessage(db_connection));					
			sleep(DB_BEFORE_RECONNECT_SLEEP_SEC);
		}
		res = PQexec(db_connection, qstring);
		PQclear(res);
		return(1);
	}
	else{
		fprintf(stderr,"%s SQL connection not defined!",
			ctime(&t));
		return(0);
	}
}

/*---------------------------------------------------*/
void insert_call(PASTERISK_DATA AstData){
	char query_buf[QUERY_BUF_SIZE];
	sprintf(query_buf,"INSERT INTO ast_calls "\
	"(unique_id,caller_id_num,ext,call_type) VALUES "\
	"('%s','%s','%s','%s'::call_types)",
	AstData->unique_id,
	AstData->caller_id_num,
	AstData->exten,
	AstData->call_type
	);
	
	query(query_buf);
}

void update_call_start(PASTERISK_DATA AstData){
	char query_buf[QUERY_BUF_SIZE];
	
	sprintf(query_buf,"UPDATE ast_calls "\
		"SET "\
		"start_time = now()::timestamp,"\
		"answer_unique_id = '%s',"\
		"ext = '%s'"\
		"WHERE unique_id = '%s'",
	AstData->unique_id,
	AstData->caller_id_num,	
	AstData->linked_id
	);
	
	query(query_buf);	
}

void update_call_start_new_api(PASTERISK_DATA AstData){
	char query_buf[QUERY_BUF_SIZE];
	
	sprintf(query_buf,"UPDATE ast_calls "\
		"SET "\
		"start_time = now()::timestamp,"\
		"caller_id_num = '%s',"\
		"ext = '%s'"\
		"WHERE unique_id = '%s'",			
	AstData->caller_id_num,
	AstData->exten,			
	AstData->unique_id
	);
	
	query(query_buf);	
}

void update_call_end(PASTERISK_DATA AstData){
	char query_buf[QUERY_BUF_SIZE];
	/*
	sprintf(query_buf,"UPDATE ast_calls "\
	"SET end_time = now()::timestamp "\
	"WHERE answer_unique_id = '%s'",
	AstData->unique_id
	);
	*/
	sprintf(query_buf,"UPDATE ast_calls "\
	"SET end_time = now()::timestamp "\
	"WHERE unique_id = '%s'",
	AstData->linked_id
	);
	
	query(query_buf);
}
