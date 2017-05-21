#ifndef CLIENT_H
#define CLIENT_H

#define MAXLINE 1024
#define MAX_CONNECT_NUM 10
#define ACCOUNT_SIZE 20
#define SERV_PORT 8888
#define CONN_PORT 9999
#define UDP_PORT  6000
#define LISTENQ 1024
#define CMD_SIZE 10
#define OPT_SIZE 30
#define SA struct sockaddr
#define max(a,b)  ((a)>(b)?(a):(b))

struct Login_info {
	int client;
	char account[ACCOUNT_SIZE];
	char sin_addr[OPT_SIZE];
	int sin_port;
};

/**
@client
select a file and send it to the socket
*/
void cli_upload(const char* filename,int sockfd);
/**
@client
download a file from server
*/
void cli_download(const char* filename, int sockfd);

/**
@client
get the content of the server's current path
*/
void cli_ls(int sockfd);
/**
@client
to find whether cmd is a right commander or not
include cd, download, upload, mkdir
*/
int cli_Iscmd(char cmd[CMD_SIZE]);
/**
@client
if a commander need a object to handle
include cd, download, upload, mkdir
*/
void cli_cmd_Up(int sockfd, char str[CMD_SIZE], char strname[OPT_SIZE]);
/**
@client
show the on-line user
*/
void cli_list(int sockfd);
void str_echo(int sockfd);
/** 
@client
update the sourse for the server
*/
void cli_update(int fd);
#endif // !CLIENT_H

