#ifndef SERVER_H
#define SERVER_H

#define LISTENQ 1024
#define MAXLINE 1024
#define PATH_LENGTH 100
#define SERV_PORT 8888
#define LISTENQ 1024
#define CONN_SETSIZE 20
#define ACCOUNT_SIZE 20
#define LIST_SIZE 20
#define CMD_SIZE 10
#define OPT_SIZE 30
#define SA struct sockaddr
#define max(a,b)  ((a)>(b)?(a):(b))

struct Login_info {
	int client;
	char account[ACCOUNT_SIZE];
	char sin_addr[OPT_SIZE];
	int sin_port;
	char bin_port[10];
	char filelist[LIST_SIZE][OPT_SIZE];
	int num;
};
struct Login_info LoginInfo[CONN_SETSIZE];
/**
@server
get the file and send it to connected client
*/
void ser_download(const char* filename, int sockfd);
/**
@server
read the file from the socket and fwrite to the file
*/
void ser_upload(const char* filename, int sockdf);
/**
@server
signal handling
*/
void ser_sig_chid(int signo);
/**
@server
display the files of the current path from server,
send the contents to client.
*/
void ser_ls(char* path, int connfd);
/**
@server
to find whether cmd is a right commander or not
include cd, download, upload, mkdir
*/
int ser_Iscmd(char cmd[10]);
/**
@server
if a commander need a object to handle
include cd, download, upload, mkdir
*/
void ser_cmd_Up(int connfd, char str[10], char strname[20], struct Login_info logininfo);
/**
@server
show the on-line user
*/
void ser_list(int connfd,struct Login_info* logininfo);
/**
@server
broadcast the message to all the online user.
*/
void ser_broadcast(int connfd,int udpfd);
/**
@server
update the sourse from the client
*/
void ser_update(int fd,int index);

void *doit(void *arg);

int ser_find(int fd,char *filename);

#endif // !SERVER_H

