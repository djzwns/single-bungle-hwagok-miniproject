#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/time.h>
#include <time.h>
#include <errno.h>

#define BUF_SIZE 100
#define MAX_CLNT 32
#define ID_SIZE 10
#define ARR_CNT 5

#include <mysql.h>
#define DB_HOST "localhost"
#define DB_USER "pi"
#define DB_PASS "raspberry"
#define DB_SHIP "sp_db"
#define DB_RFID "rfid_db"
#define TB_SHIP "sp_info"
#define TB_RFID "rfid_info"
#define CHOP(x) x[strlen(x) - 1] = '\0'

#define DEBUG
typedef struct {
  	char fd;
	char *from;
	char *to;
	char *msg;
	int len;
}MSG_INFO;

typedef struct {
	int index;;
	int fd;
    char ip[20];
	char id[ID_SIZE];
	char pw[ID_SIZE];
}CLIENT_INFO;

void * clnt_connection(void * arg);
void send_msg(MSG_INFO * msg_info, CLIENT_INFO * first_client_info);
void error_handling(char * msg);
void log_file(char * msgstr);
void getlocaltime(char * buf);

typedef struct {
	char rfid_0[4];
	char rfid_1[4];
	char rfid_2[4];
	char rfid_3[4];
} RFID_INFO;

typedef struct {
	char number[15];
	char company[20];
} SP_INFO;

MYSQL* get_sql_connection(char* db_name)
{
	MYSQL *conn = mysql_init(NULL);

	conn = mysql_real_connect(conn, DB_HOST, DB_USER, DB_PASS, db_name, 3306, NULL, 0);
	if (!conn)
	{
		printf ("connection error: %s\n", mysql_error(conn));
		perror ("mysql init");
		exit(0);
	}
	else if (conn) printf ("success mysql\n");

	return conn;
}

int check_shipping_valid(char* sp_num)
{
	MYSQL* conn = get_sql_connection(DB_SHIP);
	MYSQL_RES* res;
	MYSQL_ROW row;
	int query_stat;

	query_stat = mysql_query(conn, "select * from sp_info");
	if (query_stat != 0)
	{
		printf ("query error : %s\n", mysql_error(conn));
		exit(1);
	}

	res = mysql_store_result(conn);

	CHOP(sp_num);
	while ( (row = mysql_fetch_row(res)) != NULL )
	{
		printf ("num: %s, row: %s, %d\n", sp_num, row[0], strcmp(sp_num, row[0]));
		// check
		if (strcmp(sp_num, row[0]) == 0)
		{
			mysql_free_result(res);
			mysql_close(conn);
			return 1;
		}
		else 
		{
			continue;
		}
	}
	mysql_free_result(res);
	mysql_close(conn);
	return 0;
}

int check_card_valid(char card_id[][4])
{
	MYSQL* conn = get_sql_connection(DB_RFID);	 

	MYSQL_RES* res;
	MYSQL_ROW row;
	int query_stat;

	query_stat = mysql_query(conn, "select * from rfid_info");
	if (query_stat != 0)
	{
		printf ("query error : %s\n", mysql_error(conn));
		exit(1);
	}

	res = mysql_store_result(conn);

	CHOP(card_id[3]);
	while ( (row = mysql_fetch_row(res)) != NULL )
	{
		printf ("check card id_0: %s, row_0: %s, %d\n", card_id[0], row[1], strcmp(card_id[0], row[1]));
		printf ("check card id_1: %s, row_1: %s, %d\n", card_id[1], row[2], strcmp(card_id[1], row[2]));
		printf ("check card id_2: %s, row_2: %s, %d\n", card_id[2], row[3], strcmp(card_id[2], row[3]));
		printf ("check card id_3: %s, row_3: %s, %d\n", card_id[3], row[4], strcmp(card_id[3], row[4]));
		// card id check
		if (strcmp(card_id[0], row[1]) == 0 &&
			strcmp(card_id[1], row[2]) == 0 &&
			strcmp(card_id[2], row[3]) == 0 &&
			strcmp(card_id[3], row[4]) == 0)
		{
			mysql_free_result(res);
			mysql_close(conn);
			return 1;
		}
		else 
		{
			continue;
		}
	}

	mysql_free_result(res);
	mysql_close(conn);
	return 0;
}

void insert_shipping_info(SP_INFO* info)
{
	MYSQL* conn = get_sql_connection(DB_SHIP);
	char query[255];

	//CHOP(info->number);
	CHOP(info->company);
	sprintf (query, "insert into sp_info values ('%s', '%s')", info->number, info->company);
	printf ("query: %s\n", query);
	if (mysql_query(conn, query) != 0)
	{
		printf ("query error: %s\n", mysql_error(conn));
		return ;
	}

	mysql_close(conn);
}

void insert_card_id(char info[][4])
{
	MYSQL* conn = get_sql_connection(DB_RFID);
	char query[255];

//	CHOP(info[0]);
//	CHOP(info[1]);
//	CHOP(info[2]);
	CHOP(info[3]);
	sprintf (query, "insert into rfid_info (RFID_0, RFID_1, RFID_2, RFID_3) values ('%s', '%s', '%s', '%s')", info[0], info[1], info[2], info[3]);

	if (mysql_query(conn, query) != 0)
	{
		printf ("query error: %s\n", mysql_error(conn));
		return ;
	}

	mysql_close(conn);
}

void delete_shipping_info(char* id)
{
	MYSQL* conn = get_sql_connection(DB_SHIP);
	char query[255];

//	CHOP(id);
	sprintf (query, "DELETE FROM sp_info WHERE SP_ID='%s'", id);

	printf ("query del: %s\n", query);
	if (mysql_query(conn, query) != 0)
	{
		printf ("query error: %s\n", mysql_error(conn));
		return ;
	}

	mysql_close(conn);
}

void delete_card_id(int id)
{
	MYSQL* conn = get_sql_connection(DB_RFID);
	char query[255];

	sprintf (query, "DELETE FROM rfid_info WHERE ID = %d", id);

	if (mysql_query(conn, query) != 0)
	{
		printf ("query error: %s\n", mysql_error(conn));
		return ;
	}

	mysql_close(conn);
}
/*
void get_card_id()
{
	MYSQL* conn = get_sql_connection(DB_RFID);	 

	MYSQL_RES* res;
	MYSQL_ROW row;
	int query_stat;

	query_stat = mysql_query(conn, "select * from rfid_info");
	if (query_stat != 0)
	{
		printf ("query error : %s\n", mysql_error(conn));
		exit(1);
	}

	res = mysql_store_result(conn);
	char temp[30];
	while ( (row = mysql_fetch_row(res)) != NULL )
	{
		printf ("check card id_0: %s, row_0: %s, %d\n", card_id[0], row[1], strcmp(card_id[0], row[1]));
		printf ("check card id_1: %s, row_1: %s, %d\n", card_id[1], row[2], strcmp(card_id[1], row[2]));
		printf ("check card id_2: %s, row_2: %s, %d\n", card_id[2], row[3], strcmp(card_id[2], row[3]));
		printf ("check card id_3: %s, row_3: %s, %d\n", card_id[3], row[4], strcmp(card_id[3], row[4]));
	}

	mysql_free_result(res);
	mysql_close(conn);
}
*/

int clnt_cnt=0;
pthread_mutex_t mutx;

int main(int argc, char *argv[])
{
	int serv_sock, clnt_sock;
	struct sockaddr_in serv_adr, clnt_adr;
	int clnt_adr_sz;
	int sock_option  = 1;
	pthread_t t_id[MAX_CLNT] = {0};
	int str_len = 0;
	int i;
	char idpasswd[(ID_SIZE*2)+3];
	char *pToken;
	char *pArray[ARR_CNT]={0};
	char msg[BUF_SIZE];

	CLIENT_INFO client_info[MAX_CLNT] = {{0,-1,"","JHJ_LDP","PASSWD"}, \
			 {0,-1,"","JHJ_SMP","PASSWD"},  {0,-1,"","JHJ_LIX","PASSWD"}, \
			 {0,-1,"","JHJ_ARD","PASSWD"},  {0,-1,"","JHJ_MAN","PASSWD"}, \
			 {0,-1,"","6","PASSWD"},  {0,-1,"","7","PASSWD"}, \
			 {0,-1,"","8","PASSWD"},  {0,-1,"","9","PASSWD"}, \
			 {0,-1,"","10","PASSWD"},  {0,-1,"","11","PASSWD"}, \
			 {0,-1,"","12","PASSWD"},  {0,-1,"","13","PASSWD"}, \
			 {0,-1,"","14","PASSWD"},  {0,-1,"","15","PASSWD"}, \
			 {0,-1,"","16","PASSWD"},  {0,-1,"","17","PASSWD"}, \
			 {0,-1,"","18","PASSWD"},  {0,-1,"","19","PASSWD"}, \
			 {0,-1,"","20","PASSWD"},  {0,-1,"","21","PASSWD"}, \
			 {0,-1,"","22","PASSWD"},  {0,-1,"","23","PASSWD"}, \
			 {0,-1,"","24","PASSWD"},  {0,-1,"","25","PASSWD"}, \
			 {0,-1,"","26","PASSWD"},  {0,-1,"","27","PASSWD"}, \
			 {0,-1,"","28","PASSWD"},  {0,-1,"","29","PASSWD"}, \
			 {0,-1,"","30","PASSWD"},  {0,-1,"","31","PASSWD"}, \
			 {0,-1,"","32","PASSWD"}};

	if(argc != 2) {
		printf("Usage : %s <port>\n",argv[0]);
		exit(1);
	}
	fputs("IoT Server Start!!\n",stdout);

	if(pthread_mutex_init(&mutx, NULL))
		error_handling("mutex init error");

	serv_sock = socket(PF_INET, SOCK_STREAM, 0);

	memset(&serv_adr, 0, sizeof(serv_adr));
	serv_adr.sin_family=AF_INET;
	serv_adr.sin_addr.s_addr=htonl(INADDR_ANY);
	serv_adr.sin_port=htons(atoi(argv[1]));

	 setsockopt(serv_sock, SOL_SOCKET, SO_REUSEADDR, (void*)&sock_option, sizeof(sock_option));
	if(bind(serv_sock, (struct sockaddr *)&serv_adr, sizeof(serv_adr))==-1)
		error_handling("bind() error");

	if(listen(serv_sock, 5) == -1)
		error_handling("listen() error");

	while(1) {
		clnt_adr_sz = sizeof(clnt_adr);
		clnt_sock = accept(serv_sock, (struct sockaddr *)&clnt_adr, &clnt_adr_sz);
		if(clnt_cnt == MAX_CLNT)
       	{
       		printf("socket full\n");
			shutdown(clnt_sock,SHUT_WR);
           	continue;
		}
		else if(clnt_sock < 0)
		{
       		perror("accept()");
          	continue;
		}

		str_len = read(clnt_sock, idpasswd, sizeof(idpasswd));
		idpasswd[str_len] = '\0';

		if(str_len > 0)
		{
			i=0;
			pToken = strtok(idpasswd,"[:]");
	
			while(pToken != NULL)
			{
				pArray[i] =  pToken;
        		if(i++ >= ARR_CNT)
					break;	
				pToken = strtok(NULL,"[:]");
			}
			for(i=0;i<MAX_CLNT;i++)
			{
				if(!strcmp(client_info[i].id,pArray[0]))
				{
					if(client_info[i].fd != -1)
					{
						sprintf(msg,"[%s] Already logged!\n",pArray[0]);
						write(clnt_sock, msg,strlen(msg));
						log_file(msg);
						shutdown(clnt_sock,SHUT_WR);
#if 1   //for MCU
     					shutdown(client_info[i].fd,SHUT_WR); 
						client_info[i].fd = -1;
//						close(client_info[i].fd);
//						pthread_mutex_lock(&mutx);
//                        client_info[i].fd = -1;
//                        clnt_cnt--;
//                        pthread_mutex_unlock(&mutx);
#endif  
						break;
					}
					if(!strcmp(client_info[i].pw,pArray[1])) 
					{

						strcpy(client_info[i].ip,inet_ntoa(clnt_adr.sin_addr));
						pthread_mutex_lock(&mutx);
						client_info[i].index = i; 
						client_info[i].fd = clnt_sock; 
						clnt_cnt++;
						pthread_mutex_unlock(&mutx);
						sprintf(msg,"[%s] New connected! (ip:%s,fd:%d,sockcnt:%d)\n",pArray[0],inet_ntoa(clnt_adr.sin_addr),clnt_sock,clnt_cnt);
						log_file(msg);
						write(clnt_sock, msg,strlen(msg));

						printf ("thread create fd: %d\n", clnt_sock);
  						pthread_create(t_id+i, NULL, clnt_connection, (void *)(client_info + i));
						pthread_detach(t_id[i]);
						break;
					}
				}
			}
			if(i == MAX_CLNT)
			{
				sprintf(msg,"[%s] Authentication Error!\n",pArray[0]);
				write(clnt_sock, msg,strlen(msg));
				log_file(msg);
				shutdown(clnt_sock,SHUT_WR);
			}
		}
		else 
			shutdown(clnt_sock,SHUT_WR);

	}
	return 0;
}

void * clnt_connection(void *arg)
{
   	CLIENT_INFO * client_info = (CLIENT_INFO *)arg;
	int str_len = 0;
	int index = client_info->index;
	char msg[BUF_SIZE];
  	char to_msg[MAX_CLNT*ID_SIZE+1];
	int i=0;
	char *pToken;
   	char *pArray[ARR_CNT]={0};
	char strBuff[130]={0};

	MSG_INFO msg_info;
	CLIENT_INFO  * first_client_info;

	first_client_info = (CLIENT_INFO *)((void *)client_info - (void *)( sizeof(CLIENT_INFO) * index ));
	while(1)
	{
		memset(msg,0x0,sizeof(msg));
		str_len = read(client_info->fd, msg, sizeof(msg)-1); 
		if(str_len <= 0)
			break;

		msg[str_len] = '\0';
		pToken = strtok(msg,"[:]");
		i = 0; 
		while(pToken != NULL)
		{
			pArray[i] =  pToken;
			if(i++ >= ARR_CNT)
				break;	
			pToken = strtok(NULL,"[:]");
		}

//		printf ("fd: %d, to: %s, from: %s, msg: %s\n", client_info->fd, pArray[0], client_info->id, pArray[1]);
		msg_info.fd = client_info->fd;
		msg_info.from = client_info->id;
		msg_info.to = pArray[0];
		sprintf(to_msg,"[%s]%s",msg_info.from,pArray[1]);
		msg_info.msg = to_msg;
		msg_info.len = strlen(to_msg);

		sprintf(strBuff,"msg : [%s->%s] %s",msg_info.from,msg_info.to,pArray[1]);
		log_file(strBuff);
		send_msg(&msg_info, first_client_info);
	}

	close(client_info->fd);

	sprintf(strBuff,"Disconnect ID:%s (ip:%s,fd:%d,sockcnt:%d)\n",client_info->id,client_info->ip,client_info->fd,clnt_cnt-1);
	log_file(strBuff);

	pthread_mutex_lock(&mutx);
	clnt_cnt--;
	client_info->fd = -1;
	pthread_mutex_unlock(&mutx);

	return 0;
}

void send_msg(MSG_INFO * msg_info, CLIENT_INFO * first_client_info)
{
	int i=0;

	printf ("fd: %d, from: %s, to: %s, msg: %s\n", msg_info->fd, msg_info->from, msg_info->to, msg_info->msg);
	if(!strcmp(msg_info->to,"ALLMSG"))
	{
		for(i=0;i<MAX_CLNT;i++)
			if((first_client_info+i)->fd != -1)	
     			write((first_client_info+i)->fd, msg_info->msg, msg_info->len);
	}
	else if(!strcmp(msg_info->to,"IDLIST"))
	{
		char* idlist = (char *)malloc(ID_SIZE * MAX_CLNT);
		msg_info->msg[strlen(msg_info->msg) - 1] = '\0';
		strcpy(idlist,msg_info->msg);

		for(i=0;i<MAX_CLNT;i++)
		{
			if((first_client_info+i)->fd != -1)	
			{
				char tmp[3];
				sprintf(tmp, "%d:%s", (first_client_info + i)->fd, (first_client_info + i)->id);
				strcat(idlist, tmp);
				//strcat(idlist,(first_client_info+i)->id);
				strcat(idlist," ");
			}
		}
		strcat(idlist,"\n");
    	write(msg_info->fd, idlist, strlen(idlist));
		free(idlist);
	}
	else
		for(i=0;i<MAX_CLNT;i++)
			if((first_client_info+i)->fd != -1)	
				if(!strcmp(msg_info->to,(first_client_info+i)->id))
     				write((first_client_info+i)->fd, msg_info->msg, msg_info->len);

	if (!strcmp(msg_info->from, "JHJ_MAN") || !strcmp(msg_info->from, "JHJ_ARD"))
	{
		if (!strcmp(msg_info->to, "CARDDEL"))
		{
			char *pToken;
			pToken = strtok(msg_info->msg,"[.]");
			pToken = strtok(NULL,"[.]");
			if(pToken != NULL)
			{
				CHOP(pToken);
				int id = atoi(pToken);
				delete_card_id(id);
				delete_shipping_info(pToken);
			}
		}
	}
	if (!strcmp(msg_info->from, "JHJ_MAN"))
	{
		if (!strcmp(msg_info->to, "CARDADD"))
		{
			char *pToken;
			char card_info[4][4];
			pToken = strtok(msg_info->msg,"[.]");
			pToken = strtok(NULL,"[.]");
			int i = 0; 
			while(pToken != NULL)
			{
				strcpy(card_info[i], pToken);
				if(i++ >= 4)
					break;	
				pToken = strtok(NULL,".");
			}
			insert_card_id(card_info);
		}
/*		else if (!strcmp(msg_info->to, "TEST"))
		{
			char card[] = "1234";
			printf ("check card : %d\n", check_shipping_valid(card));

		}*/
		else if (!strcmp(msg_info->to, "CARDGET"))
		{
		}
		else if (!strcmp(msg_info->to, "SHIPADD"))
		{
			char *pToken;
			char *pArray[15];
			pToken = strtok(msg_info->msg,"[.]");
			int i = 0; 
			while(pToken != NULL)
			{
				pArray[i] = pToken;
				if(i++ >= 3)
					break;	
				pToken = strtok(NULL,".");
			}
			SP_INFO info;
			strcpy(info.number, pArray[1]);
			if (pArray[2] != NULL)
				strcpy(info.company, pArray[2]);
			printf ("num: %s, comp: %s\n", info.number, info.company);
			insert_shipping_info(&info);
		}
		else if (!strcmp(msg_info->to, "SHIPDEL"))
		{
			char *pToken;
			pToken = strtok(msg_info->msg,"[.]");
			pToken = strtok(NULL,"[.]");
			if(pToken != NULL)
			{
				delete_shipping_info(pToken);
			}
		}
	}
	else if (!strcmp(msg_info->from, "JHJ_ARD"))
	{
		// rfid valid check
		if (!strcmp(msg_info->to, "RFVALID"))
		{
			char *pToken;
			char card_info[4][4];
			pToken = strtok(msg_info->msg,"[.]");
			pToken = strtok(NULL,"[.]");
			int i = 0; 
			while(pToken != NULL)
			{
				strcpy(card_info[i], pToken);
				if(i++ >= 4)
					break;	
				pToken = strtok(NULL,".");
			}
			if (check_card_valid(card_info))
			{ 
				char result[30] = {0};
				sprintf (result, "[%s]CARD@OK\n", msg_info->from);
				usleep(50000);
				write(msg_info->fd, result, strlen(result));
			}
			else
			{
				char result[30] = {0};
				sprintf (result, "[%s]CARD@FAIL\n", msg_info->from);
				usleep(50000);
				write(msg_info->fd, result, strlen(result));
			}
		}
		else if (!strcmp(msg_info->to, "SPVALID"))
		{
			char *pToken;
			char number[20];
			pToken = strtok(msg_info->msg,"[.]");
			printf ("token: %s\n", pToken);
			pToken = strtok(NULL,"[.]");
			if (pToken != NULL)
				strcpy(number, pToken);
			printf ("SPVALID: %s\n", number);
			if (check_shipping_valid(number))
			{ 
				printf ("SPVALID OK\n");
				char result[30] = {0};
				sprintf (result, "[%s]SHIP@OK\n", msg_info->from);
				usleep(50000);
				write(msg_info->fd, result, strlen(result));
			}
			else
			{
				printf ("SPVALID FAIL\n");
				char result[30] = {0};
				sprintf (result, "[%s]SHIP@FAIL\n", msg_info->from);
				usleep(50000);
				write(msg_info->fd, result, strlen(result));
			}
		}
	}
}

void error_handling(char *msg)
{
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
}

void log_file(char * msgstr)
{
	fputs(msgstr,stdout);
}
