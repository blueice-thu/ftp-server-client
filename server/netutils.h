#ifndef _NETUTILS_H_
#define _NETUTILS_H_

#include "common.h"

// Create server
int create_ftp_server(const char *host, unsigned short port);

void receive_request(int listener_d);

void* process_request(void* client_descriptor);

void process_command(char* command, char* args, connection_state* state);

void command_user(char* args, connection_state* state);
void command_pass(char* args, connection_state* state);
void command_retr(char* args, connection_state* state);
void command_stor(char* args, connection_state* state);
void command_quit(char* args, connection_state* state);
void command_syst(char* args, connection_state* state);
void command_type(char* args, connection_state* state);
void command_port(char* args, connection_state* state);
void command_pasv(char* args, connection_state* state);
void command_mkd(char* args, connection_state* state);
void command_cwd(char* args, connection_state* state);
void command_pwd(char* args, connection_state* state);
void command_list(char* args, connection_state* state);
void command_rmd(char* args, connection_state* state);
void command_rnfr(char* args, connection_state* state);
void command_rnto(char* args, connection_state* state);
void command_abor(char* args, connection_state* state);
void command_dele(char* args, connection_state* state);

#endif