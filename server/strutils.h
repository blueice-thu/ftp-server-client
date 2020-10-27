#ifndef _STRUTILS_H_
#define _STRUTILS_H_

#include "common.h"

// Maximum number of words per line in config.conf
#define MAX_LINE_LENGTH_CONFIG 512

// Check if the program has root permission
void check_root_permission();

/**
* Read parament configs from config.conf
* @param config A config struct pointer to store
*/
void read_config();

/**
* Read paraments from opts. 
* It will overwrite config from config.conf.
* Support: -port, -root
* @param argc Number of parameters
* @param argv String list of parameters
* @param config A config struct pointer to store
*/
void get_paras(int argc, char *argv[]);

#endif