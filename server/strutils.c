#include "strutils.h"

void check_root_permission() {
    if (geteuid() != 0) {
        fprintf(stderr, "Wrong: FTP Server need root permission to work!\r\n");
        exit(EXIT_FAILURE);
    }
}

void read_config() {
    memset(&config, 0, sizeof(config));
    const char filename[] = "config.conf";
    char line[MAX_LINE_LENGTH_CONFIG] = {0};
    getcwd(config.code_path, PATH_LENGTH);
    FILE* pf = NULL;
    pf = fopen(filename, "r");
    if (pf == NULL) {
        fprintf(stderr, "Wrong: Fail to open config file config.conf\r\n");
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
            if(chdir(value) !=0 ) {
                printf("%s: %s\r\n", config.root_path, strerror(errno));
                exit(EXIT_FAILURE);
            }
            getcwd(config.root_path, PATH_LENGTH);
        }
        else if (strcmp(key, "num_user") == 0) {
            config.num_user = atoi(value);
            config.username_table = (char**)calloc(config.num_user, sizeof(char*));
            config.password_table = (char**)calloc(config.num_user, sizeof(char*));
        }
        else if (strcmp(key, "log_file") == 0) {
            int join_status = join_path(config.code_path, value, config.log_path);
            if (join_status == 0) {
                printf("Error: Wrong log path!\n");
                exit(EXIT_FAILURE);
            }
        }
        else if (strncmp(key, "user_", 5) == 0) {
            config.username_table[config.custom_num_user] = strdup(key + 5);
            config.password_table[config.custom_num_user] = strdup(value);
            config.custom_num_user += 1;
        }
    }
}

void get_paras(int argc, char *argv[]) {
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-port") == 0) {
            if (i + 1 >= argc) {
                printf("Error: Invaild parameters!\n");
                exit(EXIT_FAILURE);
            }
            config.listen_port = atoi(argv[i + 1]);
        }
        else if (strcmp(argv[i], "-root") == 0) {
            if (i + 1 >= argc) {
                printf("Error: Invaild parameters!\n");
                exit(EXIT_FAILURE);
            }
            if (argv[i + 1][0] == '/') {
                if(chdir(argv[i + 1]) != 0) {
                    printf("%s: %s\r\n", argv[i + 1], strerror(errno));
                    exit(EXIT_FAILURE);
                }
                memset(config.root_path, 0, sizeof(config.root_path));
                getcwd(config.root_path, PATH_LENGTH);
            }
            else {
                memset(config.root_path, 0, sizeof(config.root_path));
                int join_status = join_path(config.code_path, argv[i + 1], config.root_path);
                if (join_status == 0 || chdir(config.root_path) != 0) {
                    printf("%s: %s\r\n", config.root_path, strerror(errno));
                    exit(EXIT_FAILURE);
                }
            }
        }
    }
}