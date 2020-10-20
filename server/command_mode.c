#include "command_mode.h"

void command_type(char* args, Session* state) {
    if (state->is_logged == 0) {
        send_message(state, need_login_msg);
        return;
    }
    if(strcmp(args, "I") == 0) {
        send_message(state, "200 Switching to Binary mode.\r\n");
    }
    else {
        send_message(state, "501 Syntax error.\r\n");
    }
}

void command_port(char* args, Session* state) {
    if (state->is_logged == 0) {
        send_message(state, need_login_msg);
        return;
    }
    state->mode = ACTIVE;

    unsigned int ip[4] = { 0 };
    unsigned int port1 = 0, port2 = 0;
    sscanf(args, "%u,%u,%u,%u,%u,%u", &ip[0], &ip[1], &ip[2], &ip[3], &port1, &port2);

    SockAddrIn *port_addr = (SockAddrIn*)calloc(1, sizeof(SockAddrIn));
    port_addr->sin_family = AF_INET;
    
    char ip_decimal[40];
    sprintf(ip_decimal, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
    port_addr->sin_addr.s_addr = inet_addr(ip_decimal);
    int port_dec = port1 * 256 + port2;
    port_addr->sin_port = htons(port_dec);
    state->port_addr = port_addr;

    state->data_trans_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (state->data_trans_fd < 0) {
        perror("Error in socket of create_socket");
        state->data_trans_fd = -1;
        return;
    }
    send_message(state, "200 Command PORT okay.\r\n");
}

void command_pasv(char* args, Session* state) {
    if (state->is_logged == 0) {
        send_message(state, need_login_msg);
        return;
    }

    state->mode = PASSIVE;

    state->sock_pasv = create_socket(0, state);
    SockAddrIn* sock_addr = state->pasv_addr;
    socklen_t addr_size = sizeof(SockAddrIn);
    if (getsockname(state->sock_pasv, (SockAddr*)sock_addr, &addr_size) != 0) {
        send_message(state, "425 Cannot open passive connection.\r\n");
        return ;
    }

    char msg[MSG_LENGTH] = { '\0' };
    int ip[4] = { 0 };
    get_local_ip(state->sockfd, ip);
    int port = (int)(ntohs(sock_addr->sin_port));
    int port1 = port / 256;
    int port2 = port % 256;
    sprintf(msg, "227 Entering Passive Mode (%d,%d,%d,%d,%d,%d).\r\n", ip[0], ip[1], ip[2], ip[3], port1, port2);
    send_message(state, msg);
}