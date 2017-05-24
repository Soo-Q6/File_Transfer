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
#include<sys/stat.h>
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
	char *delim="-";
	char sendline[100] = { '\0' };
	printf("the current path:%s\n",getcwd(NULL,NULL));
	while ((pDir = opendir("cli_file")) == NULL)
	{
		printf("cannot open direactory.");
		write(connfd, sendline, 2);
		return;
	}
	while ((ent = readdir(pDir)) != NULL)
	{
		strcpy(sendline, ent->d_name);
		char tmp[10];
		//itoa(get_file_size("cli_file",ent->d_name),tmp,10);
		sprintf(tmp,"%d",get_file_size("cli_file",ent->d_name));
		//printf("%s\t%s\n",ent->d_name,tmp);
		if(strcmp(sendline,".")==0||strcmp(sendline,"..")==0){
			continue;
		}
		strcat(sendline,delim);
		strcat(sendline,tmp);
		printf("%s \n",sendline);
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
	else if (!strcmp(str, "catch"))
	{
		char mes[OPT_SIZE + 10];
		int usercount = 0,i;
		char *delim = "-";
		char *pch;
		int filesize;
		pthread_t tid;
		int blockcount;
		struct Catch_info c_info;
		struct File_info info[CONN_SIZE];
		send(sockfd, strname, OPT_SIZE, 0);
		while (read(sockfd, mes, sizeof(mes)) > 1)
		{
			printf("%s\n", mes);
			pch = strtok(mes, delim);
			strcpy(info[usercount].sin_addr, pch);
			pch = strtok(NULL, delim);
			info[usercount].sin_port = atoi(pch);
			info[usercount].filesize=atoi(strtok(NULL,delim));
			filesize=info[usercount].filesize;
			usercount++;
		}
		blockcount=filesize/1024+1;
		for(i=0;i<blockcount;i++){
			strcpy(c_info.file,strname);
			c_info.which_block=i;
			strcpy(c_info.sin_addr,info[i%usercount].sin_addr);
			c_info.sin_port=info[i%usercount].sin_port;
			if(pthread_create(&tid,NULL,&cli_catch,&c_info)){
				printf("%s  errno:%d",strerror(errno),errno);
			}
			//memset(&c_info,0,sizeof(c_info));
		}
	}
	else
	{
		printf("error\n");
	}
}


void *cli_listen(void *fd){
	int listenfd=*((int*)fd);
	int connfd;
	int n;
	FILE *f;
	char *delim="-";
	char recvline[MAXLINE];
	char tmp[OPT_SIZE];
	char file[OPT_SIZE];
	int size;
	chdir("cli_file");
	while((connfd=accept(listenfd,NULL,NULL))>0){
		n=read(connfd,tmp,sizeof(tmp));
		printf("this ma:%s\n",tmp);
		strcpy(file,strtok(tmp,delim));
		printf("this requirement file:%s",file);
		size=atoi(strtok(NULL,delim));
		printf("this is the which_block:%d\n",size);
		f=fopen(file,"r");
		if(f==NULL){
			printf("fopen error\n");
			return NULL;
		}
		fseek(f,size*1024,SEEK_SET);
		fread(recvline,1,MAXLINE,f);
		n=write(connfd,recvline,sizeof(recvline));
		printf("this is the sending size:n\n",n);
		fclose(f);
		close(connfd);
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

unsigned long get_file_size(char *path,char *file){
	//unsigned long filesize;
	struct stat statbuf;
	chdir(path);
	//stat(file,&statbuf);
	if(stat(file,&statbuf)<0){
		printf("%s  %d\n",strerror(errno),errno);
		chdir("..");
		return -1;
	}else{
		chdir("..");
		return statbuf.st_size;
	}
}

void *cli_catch(void *mes){
	struct Catch_info c_info=*((struct Catch_info*)mes);
	struct sockaddr_in cliaddr;
	char *delim="-";
	char tmp[OPT_SIZE];
	char recvline[MAXLINE];
	char tmp_w[5];
	int sockfd,n;
	printf("this is the block num i want to write:%d\n",c_info.which_block);
	chdir("cli_file");
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		printf("socket error");	
		return NULL;	
	}
	bzero(&cliaddr, sizeof(cliaddr));
	cliaddr.sin_family = AF_INET;
	printf("this is the connect port:%d\n",c_info.sin_port);
	cliaddr.sin_port = htons(c_info.sin_port);
	if (inet_pton(AF_INET, c_info.sin_addr, &cliaddr.sin_addr) <= 0){
		printf("inet_ption error for %s\n", c_info.sin_addr);
		return NULL;
	}
	if ((n = connect(sockfd, (SA *)&cliaddr, sizeof(cliaddr))) < 0){
		printf("connect error");
		//exit(0);
		return NULL;
	}
	strcpy(tmp,c_info.file);
	strcat(tmp,delim);
	sprintf(tmp_w,"%d",c_info.which_block);
	printf("this is which_block char:%s\n",tmp_w);
	strcat(tmp,tmp_w);
	printf("this is the message i want to send:%s\n",tmp);
	write(sockfd,tmp,sizeof(tmp));
	FILE *f=fopen(c_info.file,"w");
	n=read(sockfd,recvline,sizeof(recvline));
	printf("%d n\n",n);
	fseek(f,c_info.which_block*1024,SEEK_SET);
	fwrite(recvline,1,n,f);
	fclose(f);
	close(sockfd);
	pthread_exit(pthread_self());
	return NULL;
}