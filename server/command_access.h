#ifndef _COMMAND_ACCESS_H_
#define _COMMAND_ACCESS_H_

#include "common.h"

void command_user(char* args, Session* state);

void command_pass(char* args, Session* state);

void command_quit(char* args, Session* state);

void command_syst(char* args, Session* state);

void command_abor(char* args, Session* state);

#endif