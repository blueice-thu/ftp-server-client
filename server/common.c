#include "common.h"

Config config;

const char need_login_msg[] = "530 Need login.\r\n";
const char no_file_msg[] = "550 No such file or directory.\r\n";
const char no_permis_msg[] = "550 Permission denied.\r\n";

const char *cmdlistStr[] = 
{
    "USER", "PASS", "RETR", "STOR", "QUIT", "SYST", "TYPE", "PORT", "PASV",
    "MKD", "CWD", "PWD", "LIST", "RMD", "RNFR", "RNTO", "ABOR", "DELE", "CDUP"
};

void send_message(Session* state, const char* msg) {
    int bytes = strlen(msg);
    state->trans_all_bytes += bytes;
    state->trans_all_num += 1;
    write(state->sockfd, msg, bytes);
}

int create_socket(int port, Session* state)
{
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Error in socket of create_socket");
        return -1;
    }

    SockAddrIn *sock_addr = (SockAddrIn*)calloc(1, sizeof(SockAddrIn));
    sock_addr->sin_family = AF_INET;
    sock_addr->sin_port = htons(port);
    sock_addr->sin_addr.s_addr = htonl(INADDR_ANY);

    // Make port be able to be reused
    int reuse = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(int)) == -1) {
        close(sockfd);
        perror("Error in setsockopt of create_socket");
        return -1; 
    }

    if (bind(sockfd, (SockAddr *) sock_addr, sizeof(SockAddrIn)) < 0) {
        close(sockfd);
        perror("Error in bind of create_socket");
        return -1; 
    }
    if(listen(sockfd, SOMAXCONN) < 0) {
        close(sockfd);
        perror("Error in listen of create_socket");
        return -1;
    }
    if (state != NULL) {
        if (state->pasv_addr != NULL) free(state->pasv_addr);
        state->pasv_addr = sock_addr;
    }
    else
        free(sock_addr);
    return sockfd;
}

void get_local_ip(int* ip) {
    char ip_str[32] = { 0 };
    int listenfd = socket(AF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(80);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    inet_pton(AF_INET, "8.8.8.8", &(addr.sin_addr.s_addr));
    connect(listenfd, (struct sockaddr *)&(addr), sizeof(addr));

    socklen_t n = sizeof addr;
    getsockname(listenfd, (struct sockaddr *)&addr, &n);
    inet_ntop(AF_INET, &(addr.sin_addr), ip_str, INET_ADDRSTRLEN);
    sscanf(ip_str, "%d.%d.%d.%d", &ip[0], &ip[1], &ip[2], &ip[3]);
}

void update_data_trans_fd(Session* state) {
    if (state->mode == ACTIVE) {
        int status = connect(state->data_trans_fd, (SockAddr*)(state->port_addr), sizeof(SockAddr));
        if (status != 0) {
            if (state->data_trans_fd > 2) close(state->data_trans_fd);
            state->data_trans_fd = -1;
        }
    }
    else if (state->mode == PASSIVE) {
        state->data_trans_fd = accept(state->sock_pasv, NULL, NULL);
    }
    else {
        state->data_trans_fd = -1;
    }
}

void close_trans_conn(Session* state) {
    close(state->data_trans_fd);
    if (state->mode == PASSIVE) {
        close(state->sock_pasv);
    }
}

int search_username(const char* username) {
    int index = -1;
    for (int i = 0; i < config.custom_num_user; i++) {
        if (strcmp(username, config.username_table[i]) == 0)
            return i;
    }
    return index;
}

int check_password(int index, const char* password) {
    if (index < 0) return 0;
    if (strcmp(config.password_table[index], password) == 0)
        return 1;
    return 0;
}

void free_config() {
    free(config.listen_address);
    for (int i = 0; i < config.custom_num_user; i++) {
        free(config.username_table[i]);
        free(config.password_table[i]);
    }
    free(config.username_table);
    free(config.password_table);
}

int join_path(char* parent, char* child, char* result) {
    if (result == NULL) result = (char*)malloc(PATH_LENGTH);
    int n_child = strlen(child);
    strcpy(result, parent);
    int n_result = strlen(result), i_child = 0;
    if (result[n_result - 1] != '/' && child[0] != '/' && !(n_child >= 2 && child[0] == '.' && child[1] == '.')) {
        strcat(result, "/");
        n_result += 1;
    }
    while (i_child < n_child) {
        if (child[i_child] == '/') {
            if (result[n_result - 1] == '/') {
                i_child += 1;
            }
            else if (i_child + 2 < n_child && child[i_child + 1] == '.' && child[i_child + 2] == '.') {
                i_child += 1;
            }
            else if (i_child + 1 == n_child) {
                i_child += 1;
            }
            else {
                result[n_result++] = child[i_child++];
            }
        }
        else if (child[i_child] == '.') {
            if (i_child + 1 >= n_child) return 0;
            // "PATH + ./"
            if (child[i_child + 1] == '/') {
                if (n_result > 1) {
                    result[n_result] = '/';
                    n_result += 1;
                }
                i_child += 2;
            }
            // "PATH + ..PATH"
            else if (child[i_child + 1] == '.') {
                // "/ + ..PATH"
                if (n_result == 1) return 0;
                while (n_result >= 1 && result[n_result - 1] != '/') {
                    n_result--;
                    result[n_result] = '\0';
                }
                // "/PATH1/PATH2 + ..PATH"
                if (n_result > 1) {
                    n_result--;
                    result[n_result] = '\0';
                }
                i_child += 2;
            }
            else {
                result[n_result++] = child[i_child++];
            }
        }
        else {
            result[n_result++] = child[i_child++];
        }
    }
    return 1;
}

int is_valid_path(char* path) {
    if (strlen(path) < strlen(config.root_path)) return 0;
    if (strncmp(path, config.root_path, strlen(config.root_path)) != 0) return 0;
    return 1;
}

int get_args_full_path(Session* state, char* args, char* result) {
    char real_work_dir[PATH_LENGTH] = { '\0' };
    int join_status = join_path(config.root_path, state->work_dir, real_work_dir);
    if (join_status == 0) return 0;
    join_status = join_path(real_work_dir, args, result);
    if (join_status == 0) return 0;
    return 1;
}

void get_current_time(char time_str[]) {
    time_t timer;
    struct tm *tblock;
    time(&timer);
    tblock = gmtime(&timer);
    memset(time_str, 0, strlen(time_str));
    sprintf(time_str, "%d.%d.%d %d:%d:%d", tblock->tm_year+1900, tblock->tm_mon+1, tblock->tm_mday, tblock->tm_hour+8, tblock->tm_min, tblock->tm_sec);
}

void log_record_start() {
    int ip[4] = { 0 };
    get_local_ip(ip);
    log_record_string("=====================FTP Server start!====================");
    char buffer[1024];
    sprintf(buffer, "Listen Port: %u", config.listen_port);
    log_record_string(buffer);
    sprintf(buffer, "Local IP Address: %d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
    log_record_string(buffer);
    sprintf(buffer, "Root Path: %s", config.root_path);
    log_record_string(buffer);
}

void log_record_string(char content[]) {
    FILE* fp = fopen(config.log_path, "a");
    if (fp == NULL) {
        printf("Error: cannot open log file!\n");
        exit(EXIT_FAILURE);
    }
    char time_buffer[TIME_LENGTH] = { '\0' };
    get_current_time(time_buffer);
    fprintf(fp, "%s\t%s\n", time_buffer, content);
    fclose(fp);
}