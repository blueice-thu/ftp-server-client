#include "command_mode.h"

void command_type(char* args, Session* state) {
    if (state->logged == 0) {
        write(state->sockfd, need_login_msg, sizeof(need_login_msg));
        return;
    }
    char msg[MSG_LENGTH] = { '\0' };
    if(strcmp(args, "I") != 0) {
        strcpy(msg, "503 Wrong type.\n");
    }
    else {
        strcpy(msg, "200 Type set to I.\n");
    }
    write(state->sockfd, msg, sizeof(msg));
}

void command_port(char* args, Session* state) {
    // TODO
    if (state->logged == 0) {
        write(state->sockfd, need_login_msg, sizeof(need_login_msg));
        return;
    }
    unsigned int ip[4] = { 0 };
    unsigned int port1 = 0, port2 = 0;
    sscanf(args, "%u,%u,%u,%u,%u,%u", &ip[0], &ip[1], &ip[2], &ip[3], &port1, &port2);

    struct sockaddr_in *port_addr = (struct sockaddr_in *)malloc(sizeof (struct sockaddr_in));
    memset(port_addr, 0, sizeof(port_addr));
    port_addr->sin_family = AF_INET;
    
    char ip_decimal[40];
    sprintf(ip_decimal, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
    port_addr->sin_addr.s_addr=inet_addr(ip_decimal);
    int port_dec = port1 * 256 + port2;
    port_addr->sin_port = htons(port_dec);
    state->port_addr = port_addr;

    state->data_trans_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (connect(state->data_trans_fd, (struct sockaddr *)(state->port_addr), sizeof(struct sockaddr)) != 0) {
        printf("Try to connect %s %d.\n", inet_ntoa(port_addr->sin_addr), ntohs(port_addr->sin_port));
        send_message(state, "425 Fail to establish connection.\n");
        close(state->data_trans_fd);
    }
    else {
        state->mode = ACTIVE;
        send_message(state, "200 Command PORT okay.\n");
    }
}

void command_pasv(char* args, Session* state) {
    if (state->logged == 0) {
        write(state->sockfd, need_login_msg, sizeof(need_login_msg));
        return;
    }

    int port1 = 0, port2 = 0, port;
    srand(time(NULL));
    port1 = 128 + (rand() % 64);
    port2 = rand() % 0xff;
    port = 256 * port1 + port2;

    int sockfd = state->sockfd;
    socklen_t addr_size = sizeof(struct sockaddr_in);
    struct sockaddr_in addr;
    if (getsockname(sockfd, (struct sockaddr *)&addr, &addr_size) != 0) {
        send_message(state, "425 Cannot open passive connection.\n");
        return ;
    }
    int ip[4] = { 0 };
    int host = addr.sin_addr.s_addr;
    for (int i = 0; i < 4; i++) 
        ip[i] = (host >> i * 8) & 0xff;
    
    if (state->passive_socket > 2)
        close(state->passive_socket);
    state->passive_socket = create_socket(port);
    state->mode = PASSIVE;

    char msg[MSG_LENGTH] = { '\0' };
    sprintf(msg, "227 Entering Passive Mode (%d,%d,%d,%d,%d,%d).\n", ip[0], ip[1], ip[2], ip[3], port1, port2);
    send_message(state, msg);
}