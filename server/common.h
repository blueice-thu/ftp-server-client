#ifndef _COMMON_H_
#define _COMMON_H_

#define _XOPEN_SOURCE 500
#define _GNU_SOURCE 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h> 
#include <sys/sendfile.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <getopt.h>
#include <fcntl.h>
#include <ftw.h>
#include <sys/types.h>
#include <dirent.h>
#include <fcntl.h>
#include <errno.h>

static const char need_password_msg[] = "331 Guest login ok, send your complete e-mail address as password.\n";
static const char user_invaild_msg[] = "530 Invalid username.\n";
static const char need_login_msg[] = "530 Need login.\n";
static const char login_succeed_msg[] = "230 Login succeed.\n";
static const char wrong_password_msg[] = "530 Wrong password.\n";
static const char passive_msg[] = "227 Entering Passive Mode (%d,%d,%d,%d,%d,%d).\n";
static const char need_passive_msg[] = "550 Please Use Passive Mode.\n";
static const char open_data_conn_msg[] = "150 Read file OK and ready to open data transfer connection.\n";
static const char fail_read_file_msg[] = "451 Failed to read file.\n";
static const char send_file_ok_msg[] = "226 Send file successfully.\n";
static const char fail_tcp_conn_msg[] = "425 Fail to establish TCP connection.\n";
static const char network_fail_msg[] = "426 Network terminated.\n";

static const char no_file_msg[] = "550 No such file or directory.\n";
static const char no_permis_msg[] = "550 Permission denied.\n";

static const char username[] = "anonymous";
static const char password[] = "password";

#define BUFFER_LENGTH   1024    // Length of data buffer to transfer data
#define COMMAND_LENGTH  8       // Maximum length of command
#define ARGS_LENGTH     128     // Maximum length of parameter
#define USERNAME_LENGTH 32      // Maximum length of usename
#define PATH_LENGTH     256     // Maximum length of server path
#define MSG_LENGTH      512     // Maximum length of a message

typedef struct sockaddr_in SockAddrIn;
typedef struct sockaddr SockAddr;

typedef struct Config {
    unsigned int listen_port;
    char* listen_address;
    char* root_path;
} Config;

// Supported ftp commands
typedef enum cmdlist { 
    USER, PASS, RETR, STOR, QUIT, SYST, TYPE, PORT, PASV,
    MKD, CWD, PWD, LIST, RMD, RNFR, RNTO, ABOR, DELE, CDUP
} cmdlist;

static const char *cmdlistStr[] = 
{
    "USER", "PASS", "RETR", "STOR", "QUIT", "SYST", "TYPE", "PORT", "PASV",
    "MKD", "CWD", "PWD", "LIST", "RMD", "RNFR", "RNTO", "ABOR", "DELE", "CDUP"
};

typedef enum SessionMode { 
    NORMAL, PASSIVE, ACTIVE
} SessionMode;

// All information in a connection
typedef struct Session {
    char username[USERNAME_LENGTH];
    int is_logged;          // Login status
    SockAddrIn* sock_addr;
    int sock_pasv;
    SessionMode mode;       // Work mode restricted in SessionMode
    SockAddrIn *port_addr;
    int sockfd;             // Sockfd for transfering message
    int data_trans_fd;      // Sockfd for transfering file
    int is_trans_data;      // State of transfering data

    char* rename_from;
    
    int trans_file_num;     // File number that sended and received
    int trans_file_bytes;   // File bytes that sended and received
    int trans_all_num;      // Times of sending and receiving data
    int trans_all_bytes;    // Bytes of sending and receiving data
} Session;

/**
* Send string message by sockfd in Session. 
* And it increase trans_all_num and trans_all_bytes.
* @param msg A pointer of constant string message
*/
void send_message(Session* state, const char* msg);

/**
* Create a socket. Bind and listen.
* @param port The port to bind
* @param state A session to store sockfd and sock_addr
*/
int create_socket(int port, Session* state);

void get_local_ip(int sockfd, int* ip);

/**
* Close data_trans_fd and sock_pasv
*/
void close_trans_conn(Session* state);

#endif