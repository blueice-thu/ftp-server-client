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

    char* p = (char*)&port_addr->sin_addr.s_addr;
    p[0] = ip[0]; p[1] = ip[1]; p[2] = ip[2]; p[3] = ip[3];
    p = (char*)&port_addr->sin_port;
    p[0] = port1; p[1] = port2;

    state->port_addr = port_addr;
    state->mode = PORT;

    char msg[] = "200 Command PORT okay.\n";
    write(state->sockfd, msg, sizeof(msg));
}

// void generate_random_port(int* port1, int* port2) {
//     srand(time(NULL));
//     *port1 = 128 + (rand() % 64);
//     *port2 = rand() % 0xff;
// }

void command_pasv(char* args, Session* state) {
    // if (state->logged == 0) {
    //     write(state->sockfd, need_login_msg, sizeof(need_login_msg));
    //     return;
    // }

    // int port1 = 0, port2 = 0, port;
    // srand(time(NULL));
    // port1 = 128 + (rand() % 64);
    // port2 = rand() % 0xff;
    // // generate_random_port(&port1, &port2);
    // port = 256 * port1 + port2;

    // int sockfd = state->sockfd;
    // socklen_t addr_size = sizeof(struct sockaddr_in);
    // struct sockaddr_in addr;
    // if (getsockname(sockfd, (struct sockaddr *)&addr, &addr_size) != 0) {
    //     printf("Wrong: unknown error!\n");
    //     exit(EXIT_FAILURE);
    // }
    // int ip[4] = { 0 };
    // int host = addr.sin_addr.s_addr;
    // for (int i = 0; i < 4; i++) 
    //     ip[i] = (host >> i * 8) & 0xff;
    
    // close(state->passive_socket);
    // state->passive_socket = create_socket(port);
    // state->mode = PASSIVE;
    // printf("Passive port: %d\n", state->passive_socket);

    // char message[128] = { '\0' };
    // sprintf(message, passive_msg, ip[0], ip[1], ip[2], ip[3], port1, port2);
    // write(state->sockfd, message, sizeof(message));
    // printf("%s\n", message);
}