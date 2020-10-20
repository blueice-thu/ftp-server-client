#include "strutils.h"

void check_root_permission() {
    if (geteuid() != 0) {
        fprintf(stderr, "Wrong: FTP Server need root permission to work!\n");
        exit(EXIT_FAILURE);
    }
}

void read_config() {
    const char filename[] = "config.conf";
    char line[MAX_LINE_LENGTH_CONFIG] = {0};
    FILE* pf = NULL;
    pf = fopen(filename, "r");
    if (pf == NULL) {
        fprintf(stderr, "Wrong: Fail to open config file config.conf\n");
        exit(EXIT_FAILURE);
    }
    while (!feof(pf)) {
        fgets(line, MAX_LINE_LENGTH_CONFIG, pf);
        int length = strlen(line);

        int all_space_flag = 1;
        for (int i = 0; i < length; i++) {
            if (!isspace(line[i])) {
                all_space_flag = 0;
                break;
            }
        }
        
        char* pos = line;
        while (isspace(*pos)) pos++;

        if (length == 0 || all_space_flag || *pos == '#') continue;

        while (length > 0 && isspace(line[length - 1])) {
            line[length - 1] = '\0';
            length--;
        }
        
        char key[128] = { 0 };
        char value[128] = { 0 };
        int i = 0;
        while (*pos != '=') {
            key[i++] = *pos;
            pos++;
        }
        pos++; i = 0;
        while (*pos != '\0') {
            value[i++] = *pos;
            pos++;
        }
        
        if (strcmp(key, "listen_port") == 0)
            config.listen_port = atoi(value);
        else if (strcmp(key, "listen_address") == 0) {
            config.listen_address = strdup(value);
        }
        else if (strcmp(key, "root_path") == 0) {
            config.root_path = strdup(value);
        }
        else if (strcmp(key, "num_user") == 0) {
            config.num_user = atoi(value);
            config.username_table = (char**)calloc(config.num_user, sizeof(char*));
            config.password_table = (char**)calloc(config.num_user, sizeof(char*));
        }
        else if (strncmp(key, "user_", 5) == 0) {
            config.username_table[config.custom_num_user] = strdup(key + 5);
            config.password_table[config.custom_num_user] = strdup(value);
            config.custom_num_user += 1;
        }
        else {
            printf("Wrong: has no attribute %s!\n", key);
        }
    }

}

void get_paras(int argc, char *argv[]) {
    int opt = 0;
    while((opt=getopt_long(argc, argv, short_options, long_options,NULL))!=-1) {
        switch(opt)
        {
            case 0:break;
            case 'h': {
                printf("Usage: sudo ./ftpServer --port 12306 --root ./spb/tmp\n");
                exit(0);
            }
            case 'p': {
                config.listen_port = atoi(optarg);
                break;
            }
            case 'r': {
                if (config.root_path) free(config.root_path);
                config.root_path = strdup(optarg);
                break;
            }
        }
    }
}