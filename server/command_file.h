#ifndef _COMMAND_FILE_H_
#define _COMMAND_FILE_H_

#include "common.h"

void command_mkd(char* args, Session* state);

void command_cwd(char* args, Session* state);

void command_pwd(char* args, Session* state);

void command_list(char* args, Session* state);

void command_rmd(char* args, Session* state);

void command_rnfr(char* args, Session* state);

void command_rnto(char* args, Session* state);

void command_dele(char* args, Session* state);

void command_cdup(char* args, Session* state);

#endif