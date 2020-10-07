#ifndef _NETUTILS_H_
#define _NETUTILS_H_

#include "common.h"

#include "command_access.h"
#include "command_file.h"
#include "command_mode.h"
#include "command_trans.h"

// Create server
int create_ftp_server(const char *host, unsigned short port);

void receive_request(int listener_d);

void* process_request(void* client_descriptor);

void process_command(char* command, char* args, Session* state);


#endif