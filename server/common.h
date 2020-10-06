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

static const char welcome_msg[] = "220 Anonymous FTP server ready.\n";
static const char unknown_command_msg[] = "500 Unknown command.\n";
static const char need_password_msg[] = "331 Guest login ok, send your complete e-mail address as password.\n";
static const char user_invaild_msg[] = "530 Invalid username.\n";
static const char need_login_msg[] = "530 Need login.\n";
static const char login_succeed_msg[] = "230 Login succeed.\n";
static const char wrong_password_msg[] = "530 Wrong password.\n";
static const char passive_msg[] = "227 Entering Passive Mode (%d,%d,%d,%d,%d,%d).\n";
static const char syst_msg[] = "215 UNIX Type: L8.\n";
static const char quit_msg[] = "221-You have transferred %d bytes in %d files.\n221-Total traffic for this session was %d bytes in %d transfers.\n221-Thank you for using the FTP service on ftp.ssast.org.\n221 Goodbye.\n";
static const char need_passive_msg[] = "550 Please Use Passive Mode.\n";
static const char open_data_conn_msg[] = "150 Read file OK and ready to open data transfer connection.\n";
static const char fail_read_file_msg[] = "451 Failed to read file.\n";
static const char send_file_ok_msg[] = "226 Send file successfully.\n";
static const char fail_tcp_conn_msg[] = "425 Fail to establish TCP connection.\n";
static const char network_fail_msg[] = "426 Network terminated.\n";
static const char type_msg[] = "200 Type set to I.\n";
static const char type_wrong_msg[] = "503 Wrong type.\n";
static const char change_dir_msg[] = "250 Change directory successfully.\n";
static const char fail_chdir_msg[] = "550 Fail to change directory.\n";

static const char no_file_msg[] = "550 No such file or directory.\n";
static const char no_permis_msg[] = "550 Permission denied.\n";

static const char username[] = "anonymous";
static const char password[] = "password";

#define BUFFER_LENGTH 1024
#define COMMAND_LENGTH 8
#define ARGS_LENGTH 128
#define WELCOME_LENGTH 128
#define USERNAME_LENGTH 32
#define PATH_LENGTH 256
#define MSG_LENGTH 512

typedef enum cmdlist { 
    USER, PASS, RETR, STOR, QUIT, SYST, TYPE, PORT, PASV,
    MKD, CWD, PWD, LIST, RMD, RNFR, RNTO, ABOR, DELE
} cmdlist;

static const char *cmdlist_str[] = 
{
    "USER", "PASS", "RETR", "STOR", "QUIT", "SYST", "TYPE", "PORT", "PASV",
    "MKD", "CWD", "PWD", "LIST", "RMD", "RNFR", "RNTO", "ABOR", "DELE"
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
    char* rename_from;
} connection_state;

#endif