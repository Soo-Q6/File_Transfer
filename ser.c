#include <sys/socket.h>
#include <wait.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <stdio.h>
#include <sys/types.h>
#include <memory.h>
#include <dirent.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <errno.h>
#include "ser.h"

void ser_download(const char *filename, int sockfd)
{
	FILE *fp;
	ssize_t n;
	const char error[6] = "error";
	char buf[MAXLINE];
	chdir("ser_file");
	if ((fp = fopen(filename, "r")) == NULL)
	{
		printf("cannot open file!\n");
		write(sockfd, error, sizeof(error));
		//exit(0);
		return;
	}
again:
	while ((n = fread(buf, 1, MAXLINE, fp)) > 0)
	{
		write(sockfd, buf, MAXLINE);
		bzero(buf,sizeof(buf));
	}
	if (n < 0 && errno == EINTR)
		goto again;
	else if (n < 0)
		printf("str_echo:read error");
	else
		printf("download conplete!\n");
	write(sockfd, buf, 1);
	fclose(fp);
	return;
}

void ser_upload(const char *filename, int sockfd)
{
	char recvline[MAXLINE];
	FILE *fp = fopen(filename, "w");
	ssize_t n;
	if (fp == NULL)
	{
		printf("open file error\n");
		exit(0);
	}
	//fprintf(fp, "account:%s\taddr:%s\tport:%d", logininfo.account, logininfo.sin_addr, logininfo.sin_port);
again:
	while ((n = read(sockfd, recvline, MAXLINE)) >1)
	{
		//int i=0;
		//printf("%d\n",i);
		//i++;
		fwrite(recvline, 1, n, fp);
	}
	//printf("n:%d\n",n);
	if (n > 1)
	{
		if (strcmp(recvline, "error") == 0)
		{
			remove(filename);
			return;
		}
		else{
			fwrite(recvline, 1, n, fp);
		}
	}
	if (n < 0 && errno == EINTR)
		goto again;
	else if (n <= 0)
		printf("read error");
	else
		printf("Upload Complete!\n");
	fclose(fp);
	return;
}

void ser_update(int sockfd,int index)
{
	char recvline[100] = {'\0'};
	int n = 100,i;
	for (i=0; n == 100;i++)
	{
		n = read(sockfd, recvline, 100);
		if (n == 100)
		{
			strcpy(LoginInfo[index].filelist[i],recvline);
			LoginInfo[index].num++;
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

void ser_ls(char *tmp, int connfd)
{
	struct dirent *ent = NULL;
	DIR *pDir;
	char sendline[100] = {'\0'};
	//printf("path is:%s\n",tmp);
	while ((pDir = opendir(tmp)) == NULL)
	{
		printf("cannot open direactory.");
		write(connfd, sendline, 2);
		return;
	}
	while ((ent = readdir(pDir)) != NULL)
	{
		strcpy(sendline, ent->d_name);
		if (strcmp(sendline, ".") == 0 || strcmp(sendline, "..") == 0)
		{
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

int ser_Iscmd(char cmd[CMD_SIZE])
{
	if (!strcmp(cmd, "cd") || !strcmp(cmd, "mkdir") || !strcmp(cmd, "upload") || !strcmp(cmd, "download") || !strcmp(cmd, "catch"))
		return 1;
	else
		return 0;
}

void ser_cmd_Up(int connfd, char str[CMD_SIZE], char strname[OPT_SIZE], struct Login_info logininfo)
{
	if (strcmp(str, "download") == 0)
	{
		printf("%s %s\n", str, strname);
		ser_download(strname, connfd);
		return;
	}
	else if (strcmp(str, "upload") == 0)
	{
		ser_upload(strname, connfd);
		return;
	}
	else if (strcmp(str, "mkdir") == 0)
	{
		mkdir(strname, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
		return;
	}
	else if(!strcmp(str,"catch")){
		ser_find(connfd,strname);
	}
	else
	{
		printf("error:");
		return;
	}
}

void ser_sig_chid(int signo)
{
	pid_t pid;
	int stat;
	while ((pid = waitpid(-1, &stat, WNOHANG)) > 0)
		printf("child:%d terminated\n", pid);
	return;
}

void ser_list(int sockfd, struct Login_info *logininfo)
{
	char sendline[100] = {'\0'};
	char tmp[7] = {'\0'};
	int i1;
	for (i1 = 0; i1 < CONN_SETSIZE; i1++)
		if (logininfo[i1].client > 0)
		{
			strcat(sendline, logininfo[i1].account);
			strcat(sendline, "  ip:");
			strcat(sendline, logininfo[i1].sin_addr);
			strcat(sendline, "  port:");
			sprintf(tmp, "%d", logininfo[i1].sin_port);
			strcat(sendline, tmp);
			write(sockfd, sendline, sizeof(sendline));
			bzero(sendline, strlen(sendline));
		}
	write(sockfd, sendline, 1);
}

void ser_broadcast(int sockfd, int udpfd)
{
	char recvline[MAXLINE];
	ssize_t n;
	struct sockaddr_in addrto;
	bzero(&addrto, sizeof(struct sockaddr_in));
	addrto.sin_family = AF_INET;
	addrto.sin_addr.s_addr = htonl(INADDR_BROADCAST);
	addrto.sin_port = htons(6000);
	//inet_pton(AF_INET, "127.0.0.1", &addrto.sin_addr);
	int nlen = sizeof(addrto);
	printf("this is broadcasting\n");
again:
	if ((n = read(sockfd, recvline, MAXLINE)) == MAXLINE)
	{
		//fwrite(recvline, 1, n, fp);
		printf("broadcasting context:%s\n", recvline);
		if (sendto(udpfd, recvline, MAXLINE, 0, (struct sockaddr *)&addrto, nlen) > 0)
		{
			printf("broadcast successfully\n");
		}
	}
	if (n > 1)
	{
		if (strcmp(recvline, "error") == 0)
		{
			return;
		}
		else
		{
			if (sendto(udpfd, recvline, MAXLINE, 0, (struct sockaddr *)&addrto, nlen) > 0)
			{
				printf("broadcast successfully\n");
			}
			printf("broadcasting context:%s\n", recvline);
		}
	}
	if (n < 0 && errno == EINTR)
		goto again;
	else if (n <= 0)
		printf("read error");
	else
		printf("Upload Complete!\n");
	//fclose(fp);
	return;
}

int ser_find(int fd,char *filename){
	int count=0,i;
	char mes[OPT_SIZE+10];
	char *delim="-";
	for(i=0;i<CONN_SETSIZE;i++){
		if(LoginInfo[i].client==-1){
			continue;
		printf("%d,",i);
		}else{
			int j;
			for(j=0;j<LoginInfo[i].num;j++){
				printf("%s\n",LoginInfo[i].filelist[j]);
				if(!strcmp(filename,LoginInfo[i].filelist[j])){
					count++;
					printf("%d count\n",count);
					strcat(mes,LoginInfo[i].sin_addr);
					strcat(mes,delim);
					strcat(mes,LoginInfo[i].bin_port);
					printf("%s mes\n",mes);
					write(fd,mes,sizeof(mes));
					memset(mes,0,sizeof(mes));
					//break;
				}
			}
		}
	}
	write(fd,mes,1);
	return count;
}

void *doit(void *arg)
{
	int i=*((int*)arg);
	int sockfd;
	sockfd = LoginInfo[i].client;
	while (1)
	{
		char str[CMD_SIZE] = {'\0'};
		char strname[OPT_SIZE] = {'\0'};
		int n;
		char tmp[PATH_LENGTH];
		n = chdir("ser_file");
		getcwd(tmp, PATH_LENGTH);
		recv(sockfd, str, CMD_SIZE, 0); //获取指令 done.
		printf("str:%s\n", str);
		if (strcmp(str, "ls") == 0)
		{
			ser_ls(tmp, sockfd);
		}
		else if (strcmp(str, "list") == 0)
		{
			ser_list(sockfd, LoginInfo);
		}
		else if(strcmp(str,"update")==0){
			ser_update(sockfd,i);
			int j;
			for(j=0;j<LoginInfo[i].num;j++){
				printf("%s\t",LoginInfo[i].filelist[j]);
			}
		}
		else if (strcmp(str, "exit") == 0)
		{
			printf("disconnection form:%s  port:%d\n", LoginInfo[i].sin_addr, LoginInfo[i].sin_port);
			//close(sockfd);
			//FD_CLR(sockfd, &allset);
			memset(&LoginInfo[i],0,sizeof(LoginInfo[i]));
			printf("%s\n",LoginInfo[i].sin_addr);
			LoginInfo[i].client = -1;
			close(sockfd);
			return(NULL);
			//exit(0);
			//break;
		}
		else if (ser_Iscmd(str))
		{
			recv(sockfd, strname, OPT_SIZE, 0);
			printf("strname:%s\n", strname);
			ser_cmd_Up(sockfd, str, strname, LoginInfo[i]);
		}
		else
		{
			printf("commander wrong!\n");
		}
	}
	return(NULL);
}