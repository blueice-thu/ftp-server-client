#include "common.h"

unsigned int listen_port = 21;
char* listen_address = NULL;
char* root_path = NULL;

void send_message(Session* state, const char* msg) {
    write(state->sockfd, msg, strlen(msg));
}

int create_socket(int port)
{
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        printf("Wrong: fail to open socket!\n");
        return -1; 
    }

    struct sockaddr_in sock_addr;
    sock_addr.sin_family = AF_INET;
    sock_addr.sin_port = htons(port);
    sock_addr.sin_addr.s_addr = htonl(INADDR_ANY);        

    int reuse = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(int)) == -1) {
        close(sockfd);
        printf("Wrong: fail to set sockopt!\n");
        return -1; 
    }

    if (bind(sockfd, (struct sockaddr *) &sock_addr, sizeof(sock_addr)) < 0) {
        close(sockfd);
        printf("Wrong: fail to bind!\n");
        return -1; 
    }
    if(listen(sockfd, SOMAXCONN) < 0) {
        close(sockfd);
        printf("Wrong: fail to listen!\n");
        exit(EXIT_FAILURE);
    }
    return sockfd;
}