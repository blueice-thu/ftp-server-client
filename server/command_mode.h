#ifndef _COMMAND_MODE_H_
#define _COMMAND_MODE_H_

#include "common.h"

void command_port(char* args, Session* state);

void command_pasv(char* args, Session* state);

void command_type(char* args, Session* state);

#endif