#include<sys/types.h>
#include<sys/socket.h>
#include<time.h>
#include<netinet/in.h>
#include<unistd.h>
#include<stdio.h>
#include<string.h>
#include<arpa/inet.h>
#include<errno.h>
#include<stdlib.h>
#include<dirent.h>
#include<unistd.h>
#include"cli.h"

void cli_download(const char* filename, int sockfd) {

	char recvline[MAXLINE];
	int n;
	chdir("cli_file");
	FILE* fp = fopen(filename, "w");
	if (fp == NULL)
	{
		printf("open file error\n");
		exit(0);
	}
again:
	while ((n = read(sockfd, recvline, MAXLINE)) >1)
	{
		//fputs(recvline, stdout);
		fwrite(recvline, 1, n, fp);
	}
	if (n>1)
	{
		//fputs(recvline, stdout);
		if (strcmp(recvline, "error") == 0)
		{
			printf("no such file!\n");
			remove(filename);
			return;
		}
		else
			fwrite(recvline, 1, n, fp);
	}
	if (n < 0 && errno == EINTR)
		goto again;
	else if (n < 0)
		printf("read error");
	else
		printf("Download Complete!\n");
	fclose(fp);
	return;
}



void cli_upload(const char* filename,int sockfd) {
	FILE *fp;
	ssize_t n;
	char buf[MAXLINE];
	chdir("cli_file");
	printf("the current path:%s   %s\n",getcwd(NULL,NULL),filename);
	const char error[] = "error";
	if ((fp = fopen(filename, "r")) == NULL) {
		printf("cannot open file!\n");
		write(sockfd, error, sizeof(error));
		return;
		//exit(0);
	}
again:
	while ((n = fread(buf, 1, MAXLINE, fp))>0) {
		//buf[n]='\0';
		write(sockfd, buf, MAXLINE);
		//printf("%s",buf);
		bzero(buf,sizeof(buf));
	}
	if (n < 0 && errno == EINTR)
		goto again;
	else if (n < 0)
		printf("read error");
	else
		printf("Upload Complete!\n");
	write(sockfd, buf, 1);
	fclose(fp);
	return;
}


void cli_ls(int sockfd) {
	char recvline[100] = { '\0' };
	int n = 100;
	for (; n == 100;)
	{
		n = read(sockfd, recvline, 100);
		if (n == 100)
		{
			printf("%s\t", recvline);
		}
		else if (n == 2)
		{
			printf("cannot open this direactory!\n");
		}
		else
		{
			printf("\n");
		}
	}
	return;
}


void cli_update(int connfd){
	struct dirent* ent = NULL;
	DIR *pDir;
	char sendline[100] = { '\0' };
	while ((pDir = opendir("cli_file")) == NULL)
	{
		printf("cannot open direactory.");
		write(connfd, sendline, 2);
		return;
	}
	while ((ent = readdir(pDir)) != NULL)
	{
		strcpy(sendline, ent->d_name);
		if(strcmp(sendline,".")==0||strcmp(sendline,"..")==0){
			continue;
		}
		if (write(connfd, sendline, sizeof(sendline)) < 0)
		{
			printf("write error: %s (errno:%d)", strerror(errno), errno);
			exit(0);
		}
	}
	write(connfd, sendline, 1);
	closedir(pDir);
	return;	
}

int cli_Iscmd(char cmd[CMD_SIZE]) {
	if (!strcmp(cmd, "cd") || !strcmp(cmd, "mkdir") || !strcmp(cmd, "download") || !strcmp(cmd, "upload") || !strcmp(cmd,"catch"))
		return 1;
	else
		return 0;
}

void cli_cmd_Up(int sockfd, char str[CMD_SIZE], char strname[OPT_SIZE]) {
	if (strcmp(str, "download") == 0)
	{
		printf("%s %s\n", str, strname);
		send(sockfd, strname, OPT_SIZE, 0);
		cli_download(strname, sockfd);
		return;

	}
	else if (strcmp(str, "upload") == 0)
	{
		printf("%s %s\n", str, strname);
		send(sockfd, strname, OPT_SIZE, 0);
		cli_upload(strname,sockfd);
		return;
	}
	else if (strcmp(str, "mkdir") == 0)
	{
		printf("%s %s\n", str, strname);
		send(sockfd, strname, OPT_SIZE, 0);
		printf("create dir %s successfuly\n", strname);
		return;
	}
	else if(!strcmp(str,"catch")){
		char mes[OPT_SIZE+10];
		int index=0;
		char *delim="-";
		char *pch;
		struct File_info info[CONN_SIZE];
		send(sockfd,strname,OPT_SIZE,0);
		while (read(sockfd, mes, sizeof(mes)) > 1)
		{
			printf("%s\n",mes);
			pch = strtok(mes, delim);
			strcpy(info[index].sin_addr,pch);
			pch=strtok(NULL,delim);
			info[index].sin_port=atoi(pch);
			index++;
		}
	}
	else
	{
		printf("error\n");
	}
}

void cli_list(int sockfd){
	char recvline[100] = { '\0' };
	int n = 100;
	for (; n == 100;)
	{
		n = read(sockfd, recvline, 100);
		if (n == 100)
		{
			printf("%s\n", recvline);
		}
		else
		{
			printf("\n");
		}
	}
	return;	
}

void str_echo(int sockfd) {
	ssize_t n;
	char buf[MAXLINE];
	struct sockaddr_in clientaddr;
	socklen_t clientLen = sizeof(clientaddr);
	getpeername(sockfd, (struct sockaddr*)&clientaddr, &clientLen);
	printf("connection form:%s  port:%d\n", inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));
again:
	while ((n = read(sockfd, buf, MAXLINE)) > 0) {
		buf[n] = '\0';
		write(sockfd, buf, n);

	}
	if (n < 0 && errno == EINTR)
		goto again;
	else if (n < 0)
		printf("str_echo:read error");
}