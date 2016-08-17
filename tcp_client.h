#ifndef _TCP_CLIENT_
#define _TCP_CLIENT_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>

//#include <sys/time.h>
#include <time.h>
#include <math.h>

#ifdef WIN32
	#include <winsock2.h>
	#include <ws2tcpip.h>
#else
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
#endif

#include "db_manager_pgsql.h"

/* НОВАЯ ВЕРСИЯ asterisk!!! */
#define _NEW_API


#define RSP_EV "EVENT: "
#define RSP_EV_CEL "CEL"

/*под новое API*/
#define RSP_EV_NEW_CHAN "NEWCHANNEL"
#define RSP_EV_NEW_ST "NEWSTATE"
#define RSP_CHAN_ST "CHANNELSTATE: "
#define RSP_CHAN_ST_START_VAL "0"
#define RSP_CHAN_ST_START_OUT_VAL "5"
#define RSP_CHAN_ST_START_ANSW_VAL "6"
#define RSP_EV_HANGUP "HANGUP"
#define RSP_CAUSE "CAUSE: "
#define RSP_CAUSE_NORMAL "16"
#define RSP_CONNECTEDNUM "CONNECTEDLINENUM: "

/*старое API*/
#define RSP_EV_NAME "EVENTNAME: "
#define RSP_EV_NAME_START "CHAN_START"
#define RSP_EV_NAME_ANSWER "ANSWER"
#define RSP_EV_NAME_HANGUP "HANGUP"
#define RSP_EV_NAME_LINKEDID_END "LINKEDID_END"
#define RSP_CALLERIDNUM "CALLERIDNUM: "
#define RSP_EXTEN "EXTEN: "
#define RSP_CONTEXT "CONTEXT: "
#define RSP_UNIQUEID "UNIQUEID: "
#define RSP_LINKEDID "LINKEDID: "
#define RSP_CONTEXT_INTERNAL "FROM-INTERNAL"
#define RSP_CONTEXT_TRUNK "FROM-TRUNK"

#define RCV_BUF_SIZE 2048
#define CMD_SIZE 150
#define TCP_TIMEOUT_SEC 30
#define TCP_SLEEP_SEC 2
#define TCP_CONNECT_TIMEOUT_SEC 120

void start_tcp_client();
void DieWithError(char *errorMessage);

extern char *ast_server;
extern char *ast_user;
extern char *ast_password;
extern char *ast_port;

//extern FILE *fdlog;
#endif
