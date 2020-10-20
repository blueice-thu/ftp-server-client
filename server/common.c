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
    free(config.root_path);
    for (int i = 0; i < config.custom_num_user; i++) {
        free(config.username_table[i]);
        free(config.password_table[i]);
    }
    free(config.username_table);
    free(config.password_table);
}