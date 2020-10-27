#ifndef _STRUTILS_H_
#define _STRUTILS_H_

#include "common.h"

// Maximum number of words per line in config.conf
#define MAX_LINE_LENGTH_CONFIG 512

// Check if the program has root permission
void check_root_permission();

/**
* Read parament configs from config.conf and store in global config
*/
void read_config();

/**
* Read paraments from opts and store in global config.
* It will overwrite config from config.conf.
* Support: -port, -root
* @param argc Number of parameters
* @param argv String list of parameters
*/
void get_paras(int argc, char *argv[]);

#endif