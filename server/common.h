#ifndef _COMMON_H_
#define _COMMON_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
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

// #include "config.h"

extern unsigned int listen_port;
extern char* listen_address;
extern char* root_path;

static const char welcome_message[] = "220 Anonymous FTP server ready.\r\n";
static const char unknown_command_message[] = "500 Unknown command.\r\n";
static const char need_password_message[] = "331 Need password.\r\n";
static const char user_invaild_message[] = "530 Invalid username.\r\n";
static const char need_login_message[] = "530 Need login.\r\n";
static const char login_succeed_message[] = "230 Login succeed.\r\n";
static const char wrong_password_message[] = "530 Wrong password.\r\n";
static const char passive_message[] = "227 Entering passive mode (%d,%d,%d,%d,%d,%d).\r\n";

static const char username[] = "anonymous";
static const char password[] = "admin@admin.com";

#define BUFFER_LENGTH 1024
#define COMMAND_LENGTH 8
#define ARGS_LENGTH 128
#define WELCOME_LENGTH 128
#define USERNAME_LENGTH 32

typedef enum cmdlist 
{ 
    USER, PASS, RETR, STOR, QUIT, SYST, TYPE, PORT, PASV,
    MKD, CWD, PWD, LIST, RMD, RNFR, RNTO
} cmdlist;

static const char *cmdlist_str[] = 
{
    "USER", "PASS", "RETR", "STOR", "QUIT", "SYST", "TYPE", "PORT", "PASV",
    "MKD", "CWD", "PWD", "LIST", "RMD", "RNFR", "RNTO"
};

typedef struct connection_state {
    char username[USERNAME_LENGTH];
    int logged;
    int sockfd;
} connection_state;

#endif