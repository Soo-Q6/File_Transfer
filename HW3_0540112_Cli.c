#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <time.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#include <sys/select.h>
#include <sys/wait.h>
#include "cli.h"
//void str_cli(FILE *, int);
int main(int argc, char **argv)
{
	setvbuf(stdout, NULL, _IONBF, 0);
	int sockfd, udpfd, maxfd, n = 1, nready, len, connfd;
	int listenfd;
	int connectfd[MAX_CONNECT_NUM];
	char recvline[MAXLINE], sendline[MAXLINE];
	char account[ACCOUNT_SIZE];
	struct sockaddr_in servaddr, connaddr;
	fd_set rset;
	int cmax = 0;
	time_t ticks;
	char str[10], str_name[20];
	int on = 1;
	pthread_t tid;
	//char *ipaddr = "127.0.0.1";
	if (argc != 3)
	{
		printf("open error\n");
		exit(0);
	}

	/*initial the connectfd[]*/
	int temp;
	for (temp = 0; temp < MAX_CONNECT_NUM; temp++)
		connectfd[temp] = -1;

	/*a socket for connect server*/
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		printf("socket error");
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(SERV_PORT);
	if (inet_pton(AF_INET, argv[1], &servaddr.sin_addr) <= 0)
		printf("inet_ption error for %s", argv[1]);
	fputs("please enter your acount number:\n", stdout);
	scanf("%s", account);
	if ((n = connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr))) < 0){
		printf("connect error");
		exit(0);
	}
	n=write(sockfd,argv[2],10);
	n=write(sockfd, account, sizeof(account));
	cli_update(sockfd);
	printf("%s login successfully!\ncmd/>", account);

	/*a socket for connent client*/
	if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		printf("socket error");
	setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
	bzero(&connaddr, sizeof(connaddr));
	connaddr.sin_family = AF_INET;
	connaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	connaddr.sin_port = htons(atoi(argv[2]));

	bind(listenfd, (SA *)&connaddr, sizeof(connaddr));
	listen(listenfd, LISTENQ);
	pthread_create(&tid,NULL,&cli_listen,&listenfd);

	for (;;)
	{
		char str[CMD_SIZE];
		char strname[OPT_SIZE];
		scanf("%s", str);
		printf("%s str\n", str);
		send(sockfd, str, CMD_SIZE, 0); //传指令 done.
		if (strcmp(str, "ls") == 0)
		{
			printf("this is ls\n");
			cli_ls(sockfd);
		}
		else if (strcmp(str, "exit") == 0)
		{
			exit(0);
		}
		else if(strcmp(str,"update")==0){
			cli_update(sockfd);
		}
		else if (strcmp(str, "list") == 0)
		{
			printf("this is list\n");
			cli_list(sockfd);
		}
		else if(cli_Iscmd(str)){
			scanf("%s",strname);
			send(sockfd,strname,OPT_SIZE,0);
			cli_cmd_Up(sockfd,str,strname);
		}
		else
		{
			printf("commander wrong!\n");
		}
		printf("%s/>", "cmd");
	}
	return 0;
}
