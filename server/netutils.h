#ifndef _NETUTILS_H_
#define _NETUTILS_H_

#include "common.h"

#include "command_access.h"
#include "command_file.h"
#include "command_mode.h"
#include "command_trans.h"

// Create server and return sockfd
int create_ftp_server();

/**
* Accept new client request and create thread for every client
* @param listen_fd Sockfd listening from client
*/
void receive_request(int listen_fd);

/**
* Receive command from client
* @param client_descriptor Sockfd listening from client
*/
void* process_request(void* client_descriptor);

/**
* Execute received command
* @param command command of received message
* @param args parament of received message
* @param state Session information
*/
void process_command(char* command, char* args, Session* state);

#endif