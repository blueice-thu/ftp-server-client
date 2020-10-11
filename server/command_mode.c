#include "command_mode.h"

void command_type(char* args, Session* state) {
    if (state->logged == 0) {
        send_message(state, need_login_msg);
        return;
    }
    if(strcmp(args, "I") != 0) {
        send_message(state, "200 Type set to I.\n");
    }
    else if (strcmp(args, "A") != 0) {
        send_message(state, "200 Type set to A.\n");
    }
    else {
        send_message(state, "503 Wrong type.\n");
    }
}

void command_port(char* args, Session* state) {
    // TODO
    if (state->logged == 0) {
        send_message(state, need_login_msg);
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
        if (state->data_trans_fd > 2) close(state->data_trans_fd);
    }
    else {
        state->mode = ACTIVE;
        send_message(state, "200 Command PORT okay.\n");
    }
}

void command_pasv(char* args, Session* state) {
    if (state->logged == 0) {
        send_message(state, need_login_msg);
        return;
    }
    printf("command_pasv begin\n");

    // int port1 = 0, port2 = 0, port;
    // srand(time(NULL));
    // port1 = 128 + (rand() % 64);
    // port2 = rand() % 0xff;
    // port = 256 * port1 + port2;

    state->passive_socket = create_socket(0, state);
    struct sockaddr_in *sock_addr = state->sock_addr;
    socklen_t addr_size = sizeof(struct sockaddr_in);
    if (getsockname(state->passive_socket, (struct sockaddr *)sock_addr, &addr_size) != 0) {
        send_message(state, "425 Cannot open passive connection.\n");
        return ;
    }
    
    state->mode = PASSIVE;

    char msg[MSG_LENGTH] = { '\0' };
    int ip[4] = { 0 };
    get_local_ip(state->sockfd, ip);
    int port = (int)(ntohs(sock_addr->sin_port));
    int port1 = port / 256;
    int port2 = port % 256;
    sprintf(msg, "227 Entering Passive Mode (%d,%d,%d,%d,%d,%d).\n", ip[0], ip[1], ip[2], ip[3], port1, port2);
    printf("%s\n", msg);
    send_message(state, msg);
    printf("command_pasv end\n");
}