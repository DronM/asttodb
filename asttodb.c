#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "tcp_client.h"

/* db connection variables */
char *db_server= NULL;
char *db_user=NULL;
char *db_password=NULL;
char *db_database=NULL;
char *db_port=NULL;

/* asterisk connection variables */
char *ast_server= NULL;
char *ast_user=NULL;
char *ast_password=NULL;
char *ast_port=NULL;

//FILE *fdlog;

#define PG_DEF_PORT	"5432"
#define AST_DEF_PORT "5038"
#define LOCALHOST "localhost"

/*
*/
int main(int argc, char *argv[]){
	int c;
	while ((c = getopt (argc, argv, "U:H:W:P:D:u:h:w:p")) != -1)		
         switch (c)
           {
           case 'U':
             db_user = optarg;
             break;
           case 'H':
             db_server = optarg;
             break;
           case 'W':
             db_password = optarg;
             break;
           case 'D':
             db_database = optarg;
             break;
           case 'P':
             db_port = optarg;
             break;
			//asteriks
           case 'u':
             ast_user = optarg;
             break;
           case 'h':
             ast_server = optarg;
             break;
           case 'w':
             ast_password = optarg;
             break;
           case 'p':
             ast_port = optarg;
             break;			 
           case '?':
             if (isprint (optopt))
               fprintf (stderr, "Unknown option `-%c'.\n", optopt);
             else
               fprintf (stderr,
                        "Unknown option character `\\x%x'.\n",
                        optopt);
             return 1;
           default:
             abort ();
           }

	int error_mark = 0;
	if(!db_server) {
		db_server = LOCALHOST;
	}
	if(!db_user) {
		printf("Database user name is not defined.\n");
		error_mark = 1;
	}
	if(!db_password) {
		printf("Database user password is not defined.\n");
		error_mark = 1;
	}
	if(!db_database) {
		printf("Database name is not defined.\n");
		error_mark = 1;
	}
	if(!db_port) {
		//default
		db_port = PG_DEF_PORT;
	}
	
	//asteriks
	if(!ast_server) {
		ast_server = LOCALHOST;
	}
	if(!ast_user) {
		printf("Asteriks user name is not defined.\n");
		error_mark = 1;
	}
	if(!ast_password) {
		printf("Asteriks user password is not defined.\n");
		error_mark = 1;
	}
	if(!ast_port) {
		ast_port = AST_DEF_PORT;
	}
	
	//fdlog=fopen("log", "w");
	
	if(error_mark) {
		DieWithError("Programm is terminated due to errors.\n");
	}
	
	fprintf(stdout,"Server is started with following params:\n"\
	"Database: host=%s, user=%s, password=%s, database=%s, port=%s\n"\
	"Asteriks: host=%s, user=%s, password=%s, port=%s\n",
              db_server, db_user, db_password,db_database,db_port,
               ast_server, ast_user, ast_password,ast_port
	);
			   
	//main programm
	//#ifdef _DEBUG
	puts("Starting tcp client...");
	//#endif	
	start_tcp_client();

	exit(0);
}
