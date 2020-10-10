#ifndef _STRUTILS_H_
#define _STRUTILS_H_

#include "common.h"

#define MAX_LINE_LENGTH_CONFIG 512

// Check if the program has root permission
void check_root_permission();

// Read parament configs from config.conf
void read_config(Config* config);

// Read paraments from opts
void get_paras(int argc, char *argv[], Config* config);

#endif