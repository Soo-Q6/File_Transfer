#include <stdio.h>
#include <string.h> //strlen
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>	//close
#include <arpa/inet.h> //close
#include <sys/types.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <sys/time.h> //FD_SET, FD_ISSET, FD_ZERO macros
#include "ser.h"

int main(int argc, char **argv)
{
	//setvbuf(stdout,NULL,_IONBF,0);
	int listenfd, connfd, sockfd,i;
	pthread_t tid;
	ssize_t n;
	char buf[MAXLINE];
	socklen_t clilen;
	struct sockaddr_in cliaddr, servaddr;
	//char path[6] = "essay";
	int on = 1;

	listenfd = socket(AF_INET, SOCK_STREAM, 0);
	setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(SERV_PORT);

	bind(listenfd, (SA *)&servaddr, sizeof(servaddr));
	listen(listenfd, LISTENQ);

	for (i = 0; i < CONN_SETSIZE; i++)
	{
		LoginInfo[i].client = -1;
	}

	for (;;)
	{
		clilen = sizeof(cliaddr);
		connfd = accept(listenfd, (SA *)&cliaddr, &clilen);
		if (connfd < 0)
		{
			printf("cannot accept the client's connction!\n");
			continue;
		}
		printf("thisi s:%s\n",inet_ntoa(cliaddr.sin_addr));
		for (i = 0; i < CONN_SETSIZE; i++)
			if (LoginInfo[i].client < 0)
			{
				char tmp[10];
				LoginInfo[i].client = connfd; /* save descriptor */
				strcpy(LoginInfo[i].sin_addr,inet_ntoa(cliaddr.sin_addr));
				LoginInfo[i].sin_port = ntohs(cliaddr.sin_port);
				if((n=read(connfd,tmp,sizeof(tmp)))<0){
					printf("getting client bind port error\n");
				}else{
					tmp[n]='\0';
					LoginInfo[i].bin_port=atoi(tmp);
					//printf("port:%s\n",tmp);
				}
				if ((n=read(connfd, LoginInfo[i].account, ACCOUNT_SIZE)) < 0)
				{
					printf("getting account number error\n");
					exit(0);
				}
				else
				{
					LoginInfo[i].account[n]='\0';
					printf("%s login, its ipaddr is %s and its sin_port is %d, bin_port is %d.\n", LoginInfo[i].account, LoginInfo[i].sin_addr, LoginInfo[i].sin_port,LoginInfo[i].bin_port);
				}
				break;
			}
		if(pthread_create(&tid, NULL, &doit,  &i)){
			printf("cannot create thread\n");
			continue;
		}
	}
}