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
#include<time.h>

extern const char need_login_msg[];
extern const char no_file_msg[];
extern const char no_permis_msg[];

#define BUFFER_LENGTH   1024    // Length of data buffer to transfer data
#define COMMAND_LENGTH  8       // Maximum length of command
#define ARGS_LENGTH     128     // Maximum length of parameter
#define USERNAME_LENGTH 32      // Maximum length of usename
#define PASSWORD_LENGTH 32      // Maximum length of password
#define PATH_LENGTH     512     // Maximum length of server path
#define MSG_LENGTH      512     // Maximum length of a message
#define TIMEOUT         20      // Seconds
#define TIME_LENGTH     32

#define SUPPORTED_CMD_COUNT 20

typedef struct sockaddr_in SockAddrIn;
typedef struct sockaddr SockAddr;

typedef struct Config {
    unsigned int listen_port;
    char* listen_address;
    char root_path[PATH_LENGTH];
    char code_path[PATH_LENGTH];
    char log_path[PATH_LENGTH];
    int num_user;
    int custom_num_user;
    char** username_table;
    char** password_table;
} Config;

extern Config config;
extern const char *cmdlistStr[];

// Supported ftp commands
typedef enum cmdlist { 
    USER, PASS, RETR, STOR, QUIT, SYST, TYPE, PORT, PASV,
    MKD, CWD, PWD, LIST, RMD, RNFR, RNTO, ABOR, DELE, CDUP, REST
} cmdlist;

typedef enum SessionMode { 
    NORMAL, PASSIVE, ACTIVE
} SessionMode;

// All information in a connection
typedef struct Session {
    int user_index;
    int is_logged;          // Login status
    int sock_pasv;
    SessionMode mode;       // Work mode restricted in SessionMode
    SockAddrIn* pasv_addr;
    SockAddrIn *port_addr;
    int sockfd;             // Sockfd for transfering message
    int data_trans_fd;      // Sockfd for transfering file
    int is_trans_data;      // State of transfering data

    char rename_from[PATH_LENGTH];
    int rename_state;       // Set 1 after REFR and block other commands except RNTO

    char work_dir[PATH_LENGTH];

    int rest_pos;
    
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

/**
* Create a socket. Bind and listen.
* @param ip A 4-integer list to store ip
*/
void get_local_ip(int* ip);

/**
* Update Session.data_trans_fd according to work mode
*/
void update_data_trans_fd(Session* state);

/**
* Close data_trans_fd and sock_pasv
*/
void close_trans_conn(Session* state);

/**
* Search username from username_table given by config.conf. 
* Return -1 if failing.
*/
int search_username(const char* username);

/**
* Check password of given username
* @param index The index of username
* @param password Given password to verify
*/
int check_password(int index, const char* password);

// Free the memory of pointers in config
void free_config();

/**
* Join parent and child paths CORRECTLY and store result. 
* Example 1: parent = "/tmp1", child = "/tmp2" => result = "/tmp1/tmp2". 
* Example 2: parent = "/tmp1/tmp2", child = "../tmp3" => result = "/tmp1/tmp3".
* @param parent Path prefix
* @param child Path suffix
* @param result If NULL, this function will apply a memory for result
*/
int join_path(char* parent, char* child, char* result);

// Check whether path is allowed access according to root_path
int is_valid_path(char* path);

// Generate full path by joining root_path, work_dir and args
int get_args_full_path(Session* state, char* args, char* result);

// Store "yyyy.mm.dd hh:mm:ss" in time_str
void get_current_time(char time_str[]);

// Store start information in log file
void log_record_start();

// Store a string in log file and add time prefix automatically
void log_record_string(char content[]);

#endif