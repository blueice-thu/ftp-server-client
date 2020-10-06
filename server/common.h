#ifndef _COMMON_H_
#define _COMMON_H_

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

// #include "config.h"

extern unsigned int listen_port;
extern char* listen_address;
extern char* root_path;

static const char welcome_message[] = "220 Anonymous FTP server ready.\r\n";
static const char unknown_command_message[] = "500 Unknown command.\r\n";
static const char need_password_message[] = "331 Guest login ok, send your complete e-mail address as password.\r\n";
static const char user_invaild_message[] = "530 Invalid username.\r\n";
static const char need_login_message[] = "530 Need login.\r\n";
static const char login_succeed_message[] = "230 Login succeed.\r\n";
static const char wrong_password_message[] = "530 Wrong password.\r\n";
static const char passive_message[] = "227 Entering Passive Mode (%d,%d,%d,%d,%d,%d).\r\n";
static const char syst_message[] = "215 UNIX Type: L8.\r\n";
static const char quit_message[] = "221-You have transferred %d bytes in %d files.\n221-Total traffic for this session was %d bytes in %d transfers.\n221-Thank you for using the FTP service on ftp.ssast.org.\r\n221 Goodbye.\r\n";
static const char need_passive_message[] = "550 Please Use Passive Mode.\r\n";
static const char open_data_conn_message[] = "150 Read file OK and ready to open data transfer connection.\r\n";
static const char fail_read_file_message[] = "451 Failed to read file.\r\n";
static const char send_file_ok_message[] = "226 Send file successfully.\r\n";
static const char fail_tcp_conn_message[] = "425 Fail to establish TCP connection.\r\n";
static const char network_fail_message[] = "426 Network terminated.\r\n";

static const char username[] = "anonymous";
static const char password[] = "password";

#define BUFFER_LENGTH 1024
#define COMMAND_LENGTH 8
#define ARGS_LENGTH 128
#define WELCOME_LENGTH 128
#define USERNAME_LENGTH 32

typedef enum cmdlist { 
    USER, PASS, RETR, STOR, QUIT, SYST, TYPE, PORT, PASV,
    MKD, CWD, PWD, LIST, RMD, RNFR, RNTO, ABOR
} cmdlist;

static const char *cmdlist_str[] = 
{
    "USER", "PASS", "RETR", "STOR", "QUIT", "SYST", "TYPE", "PORT", "PASV",
    "MKD", "CWD", "PWD", "LIST", "RMD", "RNFR", "RNTO", "ABOR"
};

typedef enum conn_mode { 
    NORMAL, PASSIVE, ACTIVE
} conn_mode;

typedef struct connection_state {
    char username[USERNAME_LENGTH];
    int logged;
    int sockfd;
    int passive_socket;
    conn_mode mode;
} connection_state;

#endif