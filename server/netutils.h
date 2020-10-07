#ifndef _NETUTILS_H_
#define _NETUTILS_H_

#include "common.h"

// Create server
int create_ftp_server(const char *host, unsigned short port);

void receive_request(int listener_d);

void* process_request(void* client_descriptor);

void process_command(char* command, char* args, Session* state);

void command_user(char* args, Session* state);
void command_pass(char* args, Session* state);
void command_retr(char* args, Session* state);
void command_stor(char* args, Session* state);
void command_quit(char* args, Session* state);
void command_syst(char* args, Session* state);
void command_type(char* args, Session* state);
void command_port(char* args, Session* state);
void command_pasv(char* args, Session* state);
void command_mkd(char* args, Session* state);
void command_cwd(char* args, Session* state);
void command_pwd(char* args, Session* state);
void command_list(char* args, Session* state);
void command_rmd(char* args, Session* state);
void command_rnfr(char* args, Session* state);
void command_rnto(char* args, Session* state);
void command_abor(char* args, Session* state);
void command_dele(char* args, Session* state);
void command_cdup(char* args, Session* state);

#endif