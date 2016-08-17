#include "tcp_client.h"

int param_val(char *s, char *par_name, char *block_end, char **out_val, int *out_len){
	*out_val = strstr(s,par_name);
	if (*out_val==NULL||*out_val>block_end){
		return 0;
	}
	
	*out_val+= strlen(par_name);
	
	char *par_end = strstr(*out_val,"\x0D\x0A");
	if (par_end==NULL||par_end>block_end){
		return 0;
	}
	
	*out_len = par_end - *out_val;
	return 1;
}

int valcmp(char *val,int val_len,char * pat){
	int i = 0;
	while(i<val_len){
		if (val[i]!=pat[i]){
			return 1;
		}
		i++;
	}
	return 0;
}

void parse_response(char *response){
	char *block_end;
	char *resp = response;
	int block_len;
	char *val;
	char *context_val;
	char *unique_id_val;
	char *linked_id_val;
	int val_len;
	int context_val_len;
	int unique_id_val_len;	
	int linked_id_val_len;	
	ASTERISK_DATA AstData;
	
	time_t t;
	time(&t);					
	
	int i = 0;
	while(response[i]){
      response[i] = toupper(response[i]);
      i++;
	}	
		
	while (block_end = strstr(resp,"\x0D\x0A\x0D\x0A")){		
		block_len = block_end - resp + 4;
		
		#ifdef _NEW_API
			/* под новый API*/
			if (param_val(resp, RSP_EV, block_end, &val, &val_len)==1
			&&param_val(resp, RSP_UNIQUEID, block_end, &unique_id_val, &unique_id_val_len)==1){
				/*Event exists
				&&UniqueId exists
				*/
				if (unique_id_val_len>UNIQUE_ID_MAX_LEN){
					fprintf(stderr,"%s Unique id length exeeded max value!\n",
						ctime(&t));
					return;
				}

				#ifdef _DEBUG
				puts("Новое API Event exists&&UniqueId exists.\n");
				#endif	
				
				/*
				ВХОДЯЩИЙ/ИСХОДЯЩИЙ ЗВОНОК
				Event:newchannel
				&&ChannelState exists
				&&ChannelState= RSP_CHAN_ST_START_VAL(0)
				&&Context exists
				&&Context=from-internal/from-trunk
				&&Exten exists
				&&Exten not empty!!!
				*/				
				if (valcmp(val,val_len,RSP_EV_NEW_CHAN)==0
				&&param_val(resp, RSP_CHAN_ST, block_end, &val, &val_len)==1
				&&valcmp(val,val_len,RSP_CHAN_ST_START_VAL)==0
				&&param_val(resp, RSP_CONTEXT, block_end, &context_val, &context_val_len)==1
				&&( valcmp(context_val,context_val_len,RSP_CONTEXT_TRUNK)==0 || valcmp(context_val,context_val_len,RSP_CONTEXT_INTERNAL)==0)
				&&param_val(resp, RSP_EXTEN, block_end, &val, &val_len)==1
				&&val_len
				){	
					//exten
					if (val_len>EXTEN_MAX_LEN){
						fprintf(stderr,"%s Exten length exeeded max value!",
							ctime(&t));
						return;
					}					
					strncpy(AstData.exten,val,val_len);
					AstData.exten[val_len] = '\0';

					//uniqueId
					strncpy(AstData.unique_id,unique_id_val,unique_id_val_len);
					AstData.unique_id[unique_id_val_len] = '\0';
				
					/* ОПРЕДЕЛЕНИЕ ТИПА ЗВОНКА in/out
					Context=internal = out call
					context=trunk = in call
					*/				
					if (valcmp(context_val,context_val_len,RSP_CONTEXT_TRUNK)==0){
						strcpy(AstData.call_type,"in");
						AstData.call_type[2] = '\0';
					}				
					else{
						strcpy(AstData.call_type,"out");
						AstData.call_type[3] = '\0';
					}					
				
					#ifdef _DEBUG
					puts("Входящий/Исходящий звонок.\n");
					#endif	
													
					//caller_id_num
					if (param_val(resp, RSP_CALLERIDNUM, block_end, &val, &val_len)==1){
						strncpy(AstData.caller_id_num,val,val_len);
						AstData.caller_id_num[val_len] = '\0';
					}
					
					insert_call(&AstData);
				}
				
				/*
				ЗАВЕРШЕНИЕ ЗВОНКА ВХОДЯЩИЙ/ИСХОДЯЩИЙ
				Event: Hangup
				Cause exists
				Cause=16
				*/				
				else if (valcmp(val,val_len,RSP_EV_HANGUP)==0
				&&param_val(resp, RSP_CAUSE, block_end, &val, &val_len)==1
				&&valcmp(val,val_len,RSP_CAUSE_NORMAL)==0
				){	
					#ifdef _DEBUG
					puts("Завершение звонка.\n");
					#endif	
								
					//uniqueId
					strncpy(AstData.linked_id,unique_id_val,unique_id_val_len);
					AstData.linked_id[unique_id_val_len] = '\0';
					
					update_call_end(&AstData);
					
				}
				
				/*
				НАЧАЛО РАЗГОВОРА ВХОД/ИСХОД
				Event
				&&Event=Newstate
				&&CallerIdNum exists
				*/
				else if (valcmp(val,val_len,RSP_EV_NEW_ST)==0
				&&param_val(resp, RSP_CHAN_ST, block_end, &val, &val_len)==1
				&&valcmp(val,val_len,RSP_CHAN_ST_START_ANSW_VAL)==0
				&&param_val(resp, RSP_CALLERIDNUM, block_end, &val, &val_len)==1
				){
					/* Event: newstate
					&&ChannelState exists
					&&ChannelState=6
					&&CallerIdNumExists
					*/
					#ifdef _DEBUG
					puts("Начало разговора.\n");
					#endif	
					
					/*Начало разговора вход/исход*/
					/*CallerIdNum exists*/
					if (val_len>CALLER_ID_MAX_LEN){
						fprintf(stderr,"%s Caller id num length exeeded max value!",
							ctime(&t));
						return;
					}				
					strncpy(AstData.caller_id_num,val,val_len);
					AstData.caller_id_num[val_len] = '\0';
					
					if (param_val(resp, RSP_CONNECTEDNUM, block_end, &val, &val_len)==1
					&&strcmp(AstData.caller_id_num,val)!=0
					){
						/*ConnectedLineNum exists
						&& CallerIdNum!=ConnectedLineNumber*/
						
						if (val_len>EXTEN_MAX_LEN){
							fprintf(stderr,"%s Exten length exeeded max value!",
								ctime(&t));
							return;
						}						
						strncpy(AstData.exten,val,val_len);
						AstData.exten[val_len] = '\0';
						
						//uniqueId
						strncpy(AstData.unique_id,unique_id_val,unique_id_val_len);
						AstData.unique_id[unique_id_val_len] = '\0';

						#ifdef _DEBUG
						puts("НАЧАЛО РАЗГОВОРА.\n");
						#endif	
														
						//if (strlen(AstData.caller_id_num)>val_len){
						update_call_start_new_api(&AstData);								
					}
				}
				
			}
		#else
			if (param_val(resp, RSP_EV, block_end, &val, &val_len)==1
			&&valcmp(val,val_len,RSP_EV_CEL)==0
			&&param_val(resp, RSP_EV_NAME, block_end, &val, &val_len)==1
			&&param_val(resp, RSP_CONTEXT, block_end, &context_val, &context_val_len)==1
			&&param_val(resp, RSP_UNIQUEID, block_end, &unique_id_val, &unique_id_val_len)==1
			&&param_val(resp, RSP_LINKEDID, block_end, &linked_id_val, &linked_id_val_len)==1){
				/*Event exists&&Event=CEL&&EventName exists
				&&Context exists
				&&UniqueId exists
				&&LinkedId exists
				*/
				if (unique_id_val_len>UNIQUE_ID_MAX_LEN){
					fprintf(stderr,"%s Unique id length exeeded max value!\n",
						ctime(&t));
					return;
				}
				if (linked_id_val_len>UNIQUE_ID_MAX_LEN){
					fprintf(stderr,"%s Linked id length exeeded max value!\n",
						ctime(&t));
					return;
				}
			
				if (valcmp(val,val_len,RSP_EV_NAME_START)==0){
				
					strncpy(AstData.unique_id,unique_id_val,unique_id_val_len);
					AstData.unique_id[unique_id_val_len] = '\0';
				
					strncpy(AstData.linked_id,linked_id_val,linked_id_val_len);
					AstData.linked_id[linked_id_val_len] = '\0';
				
					//unique_id==linked_id
					if (strcmp(AstData.unique_id,AstData.linked_id)==0){
					
						//caller_id_num
						if (param_val(resp, RSP_CALLERIDNUM, block_end, &val, &val_len)==1){
							if (val_len>CALLER_ID_MAX_LEN){
								fprintf(stderr,"%s Caller id num length exeeded max value!",
									ctime(&t));
								return;
							}
							strncpy(AstData.caller_id_num,val,val_len);
							AstData.caller_id_num[val_len] = '\0';
						}

						//exten
						if (param_val(resp, RSP_EXTEN, block_end, &val, &val_len)==1){
							if (val_len>EXTEN_MAX_LEN){
								fprintf(stderr,"%s Exten length exeeded max value!",
									ctime(&t));
								return;
							}
							strncpy(AstData.exten,val,val_len);
							AstData.exten[val_len] = '\0';
						}
					
					
						/* ОПРЕДЕЛЕНИЕ ТИПА ЗВОНКА in/out
						Context=internal && linked=unique_id = out call
						context=trunk && linked=unique_id in call
						*/
					
						AstData.call_type[0] = '\0';
					
						if (valcmp(context_val,context_val_len,RSP_CONTEXT_TRUNK)==0){
							strcpy(AstData.call_type,"in");
						}				
						else{
							strcpy(AstData.call_type,"out");
						}					
						insert_call(&AstData);
					}
				
				}
				else if (valcmp(val,val_len,RSP_EV_NAME_ANSWER)==0){
					strncpy(AstData.linked_id,linked_id_val,linked_id_val_len);
					AstData.linked_id[linked_id_val_len] = '\0';
				
					strncpy(AstData.unique_id,unique_id_val,unique_id_val_len);
					AstData.unique_id[linked_id_val_len] = '\0';

					/* разные unique_id и linked_id */
					if (strcmp(AstData.unique_id,AstData.linked_id)!=0){
						#ifdef _DEBUG
						fprintf(stdout,"%s unique_id=%s, linked_id=%s\n",
							ctime(&t),
							AstData.unique_id,AstData.linked_id);
						#endif
					
						if (param_val(resp, RSP_CALLERIDNUM, block_end, &val, &val_len)==1){
							if (val_len>CALLER_ID_MAX_LEN){
								fprintf(stderr,"%s Caller id num length exeeded max value!",
									ctime(&t));
								return;
							}
							strncpy(AstData.caller_id_num,val,val_len);
							AstData.caller_id_num[val_len] = '\0';
						}				
				
				
						update_call_start(&AstData);
					}
				}
				//RSP_EV_NAME_HANGUP
				else if (valcmp(val,val_len,RSP_EV_NAME_LINKEDID_END)==0){
					strncpy(AstData.unique_id,unique_id_val,unique_id_val_len);
					AstData.unique_id[unique_id_val_len] = '\0';

					strncpy(AstData.linked_id,linked_id_val,linked_id_val_len);
					AstData.linked_id[linked_id_val_len] = '\0';
				
					update_call_end(&AstData);
				}			
			}
		#endif
		resp+= block_len;		
	}
}

void send_to_socket(int sock, char *cmd){
	time_t t;
	time(&t);					
	
	int cmd_len = strlen(cmd);
	if (write(sock,cmd,cmd_len)!=cmd_len)
		fprintf(stderr,"%s write() sent a different number of bytes than expected",
			ctime(&t));		
}

int get_from_socket(int sock, char *response, char *end_marker){
	int b = 0;
	int b_total = 0;
	char *ptr;	
	
	ptr = response;
	
	memset(response, 0, RCV_BUF_SIZE);
	/*
	выходим из цикла по условиям:
		1 нашли маркер конца
		2 заполнили буфер
		3 таймаут
	*/
	while(1){
		b = read(sock, ptr, RCV_BUF_SIZE-b_total-1);
		if(b>0){
			b_total+=b;
			/*проверяем маркер конца
			и заполняемость буфера
			*/
			if (
			(strstr(response,end_marker)!=NULL)
			||(b_total==(RCV_BUF_SIZE-1))
			){
				break;
			}
			//двигаем указатель
			ptr+=b;
		}
		else {
			break;
		}		
	}
	
	return b_total;
}

void start_tcp_client(){
	int ast_sock;	
	struct sockaddr_in ast_addr;
	char cmd[CMD_SIZE];
	int cmd_len;
	char response[RCV_BUF_SIZE];
	struct timeval tvSockTimeOut;
	char *END_MARKER = "\x0D\x0A\x0D\x0A";
	
	time_t t;
	time(&t);					
	
#ifdef WIN32
	WSADATA wsaData;   // if this doesn't work
	// MAKEWORD(1,1) for Winsock 1.1, MAKEWORD(2,0) for Winsock 2.0:
	if (WSAStartup(MAKEWORD(2,2), &wsaData) != 0) {
		fprintf(stderr, "WSAStartup failed.\n");
		exit(1);
	}
#endif

sock_init:
	
	#ifdef _DEBUG	
	fprintf(stdout,"%s Creating socket\n",
		ctime(&t));
	#endif	

	ast_sock = socket(AF_INET,SOCK_STREAM,0);
	while (ast_sock<0){
		fprintf(stderr,"%s socket() failed!\n",
			ctime(&t));
		sleep(3);
		ast_sock = socket(AF_INET,SOCK_STREAM,0);
	}

	memset(&ast_addr, 0, sizeof(ast_addr));
	ast_addr.sin_family			= AF_INET;
	ast_addr.sin_addr.s_addr	= inet_addr(ast_server);
	ast_addr.sin_port			= htons(atoi(ast_port));
	
	#ifdef _DEBUG	
	fprintf(stdout,"%s Connecting to socket\n",
		ctime(&t));
	#endif	
	
	//Временный таймаут для сокета для соединения
	tvSockTimeOut.tv_sec = TCP_CONNECT_TIMEOUT_SEC;
	tvSockTimeOut.tv_usec = 0;
	setsockopt(ast_sock, SOL_SOCKET, SO_RCVTIMEO, (char *)&tvSockTimeOut,sizeof(struct timeval));	
	
	while (connect(ast_sock, (struct sockaddr *) &ast_addr, sizeof(struct sockaddr_in))<0){
		fprintf(stderr,"%s connect() failed!\n",
			ctime(&t));
		sleep(TCP_SLEEP_SEC);
	}
	#ifdef _DEBUG
	fprintf(stdout,"%s Connected to socket!\n",
		ctime(&t));
	#endif	
	
	//рабочий таймаут для сокета
	tvSockTimeOut.tv_sec = TCP_TIMEOUT_SEC;
	tvSockTimeOut.tv_usec = 0;  // Not init'ing this can cause strange errors
	setsockopt(ast_sock, SOL_SOCKET, SO_RCVTIMEO, (char *)&tvSockTimeOut,sizeof(struct timeval));	
	
	//Greeting string from Asterisk
	get_from_socket(ast_sock, response, "\x0D\x0A");
	
	if (strstr(response,"Asterisk Call Manager")==NULL){
		DieWithError("Connection problem!");
	}
	
	//Asteriks login
	#ifdef _DEBUG
	puts("Loginning to asteriks\n");
	#endif	
	
	sprintf(cmd,"Action: login%c%cUsername: %s%c%cSecret: %s%c%cEvents: off%c%c%c%c\0",
		0x0D,0x0A,ast_user,0x0D,0x0A,
		ast_password,0x0D,0x0A,
		0x0D,0x0A,0x0D,0x0A);
	send_to_socket(ast_sock, cmd);
	
	get_from_socket(ast_sock, response, END_MARKER);
	if (strstr(response,"Message: Authentication accepted")==NULL){
		#ifdef _DEBUG
		puts(response);
		#endif			
		DieWithError("Authentication error!");
	}

	//Events
	sprintf(cmd,"Action: Events%c%cEventMask: call%c%c%c%c\0",
	0x0D,0x0A,0x0D,0x0A,0x0D,0x0A);
	send_to_socket(ast_sock, cmd);	
	get_from_socket(ast_sock, response, END_MARKER);
	if (strstr(response,"Response: Success")==NULL){
		#ifdef _DEBUG
		puts(response);
		#endif			
		DieWithError("Evets setting error!");
	}

    // прием событий
	#ifdef _DEBUG
	puts("Connecting to database.\n");
	#endif	
	
	connect_to_db(db_server,
				db_port,
				db_user,
				db_password,
				db_database	
	);
	
    // прием событий
	#ifdef _DEBUG
	puts(response);
	#endif	

	puts("Starting loop");
	
    while(1){		
		if (get_from_socket(ast_sock, response, END_MARKER)>0){			
			#ifdef _DEBUG
			puts("**** DATA BEGIN ****\n");
			puts(response);
			puts("**** DATA END ****\n");
			puts("Parsing...\n");
			#endif	
			
			parse_response(response);
		}
		else{
			/*нет данных  таймаут или ошибка
			пробуем ping
			*/
			#ifdef _DEBUG	
			puts("Sending Ping...\n");
			#endif	
			
			sprintf(cmd, "Action: Ping%c%c%c%c\0",
			0x0D,0x0A,0x0D,0x0A);
			if (write(ast_sock,cmd,strlen(cmd))<=0){
				//нет ответа реинициализация сокета
				goto sock_reinit;					
			}
			if (
			!get_from_socket(ast_sock, response, END_MARKER)
			||strstr(response,"Pong")==NULL){
				//нет ответа реинициализация сокета
				goto sock_reinit;
			}
		}
	}
	
sock_reinit:	
	fprintf(stdout,"%s Socket restart!",
		ctime(&t));
	#ifdef WIN32
	  closesocket(ast_sock);
	#else
	  close(ast_sock);
	#endif
	goto sock_init;
	/*
	#ifdef WIN32
		WSACleanup();
	#endif	
	*/
}

void DieWithError(char *errorMessage){
	time_t t;
	time(&t);					
	
	fprintf(stderr,errorMessage,
		ctime(&t));
	exit(1);
}
