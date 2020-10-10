#include "common.h"

// unsigned int listen_port = 21;
// char* listen_address = NULL;
// char* root_path = NULL;

void send_message(Session* state, const char* msg) {
    write(state->sockfd, msg, strlen(msg));
}

int create_socket(int port, Session* state)
{
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        printf("Wrong: fail to open socket!\n");
        return -1; 
    }

    struct sockaddr_in *sock_addr = (struct sockaddr_in *)malloc(sizeof (struct sockaddr_in));
    sock_addr->sin_family = AF_INET;
    sock_addr->sin_port = htons(port);
    sock_addr->sin_addr.s_addr = htonl(INADDR_ANY);

    int reuse = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(int)) == -1) {
        close(sockfd);
        printf("Wrong: fail to set sockopt!\n");
        return -1; 
    }

    if (bind(sockfd, (struct sockaddr *) sock_addr, sizeof(struct sockaddr_in)) < 0) {
        close(sockfd);
        printf("Wrong: fail to bind!\n");
        return -1; 
    }
    if(listen(sockfd, SOMAXCONN) < 0) {
        close(sockfd);
        printf("Wrong: fail to listen!\n");
        exit(EXIT_FAILURE);
    }
    if (state != NULL) {
        if (state->sock_addr != NULL) free(state->sock_addr);
        state->sock_addr = sock_addr;
    }
    else
        free(sock_addr);
    return sockfd;
}

void get_local_ip(int sockfd, int* ip) {
    struct sockaddr_in addr;
    socklen_t addr_size = sizeof(struct sockaddr_in);
    getsockname(sockfd, (struct sockaddr *)&addr, &addr_size);
    int host = addr.sin_addr.s_addr;
    for (int i = 0; i < 4; i++) 
        ip[i] = (host >> i * 8) & 0xff;
}

void close_trans_conn(Session* state) {
    close(state->data_trans_fd);
    if (state->mode == PASSIVE) {
        close(state->passive_socket);
    }
}