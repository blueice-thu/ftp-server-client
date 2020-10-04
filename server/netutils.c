#include "netutils.h"

int create_ftp_server(const char *host, unsigned short port) {
    if(chroot(root_path) !=0 ) {
       printf("Wrong: cannot find root path!\n");
       exit(EXIT_FAILURE);
    }
    int listener_d = socket(PF_INET, SOCK_STREAM, 0);
    if (listener_d < 0) {
        printf("Wrong: fail to open socket!\n");
        exit(EXIT_FAILURE);
    }
    struct sockaddr_in server;
    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    if (host != NULL) {
        if(inet_aton(host, &server.sin_addr) == 0)
        {
            struct hostent *hp;
            hp = gethostbyname(host);
            if(hp == NULL) {
                printf("Wrong: invaild server address!\n");
                exit(EXIT_FAILURE);
            }
            server.sin_addr = *(struct in_addr*)hp->h_addr_list[0];
        }
    }
    else
        server.sin_addr.s_addr = htonl(INADDR_ANY);
    
    int reuse = 1;
    if((setsockopt(listener_d, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuse, sizeof(reuse))) < 0) {
        printf("Wrong: fail to set sockopt!\n");
        exit(EXIT_FAILURE);
    }
    if(bind(listener_d, (struct sockaddr*)&server, sizeof(server)) < 0) {
        printf("Wrong: fail to bind!\n");
        exit(EXIT_FAILURE);
    }
    if(listen(listener_d, SOMAXCONN) < 0) {
        printf("Wrong: fail to listen!\n");
        exit(EXIT_FAILURE);
    }

    return listener_d;
}

void receive_request(int listener_d) {
    struct sockaddr_in client_address;
    socklen_t length = sizeof(client_address);
    while(1){
        int* client_descriptor = (int*)malloc(sizeof(int));
        *client_descriptor = accept(listener_d, (struct sockaddr*)&client_address, &length);

        pthread_t pid;
        pthread_create(&pid, NULL, (void*)process_request, (void*) (client_descriptor));
    }
}

void* process_request(void* client_descriptor) {
    chdir("/");
    int sockfd = *(int*)client_descriptor;

    char command[COMMAND_LENGTH] = {'\0'};
    char args[ARGS_LENGTH] = {'\0'};
    char buffer[BUFFER_LENGTH] = {'\0'};

    connection_state* state = (connection_state*)malloc(sizeof(connection_state));
    state->logged = 0;
    state->sockfd = sockfd;
    memset(state->username, '\0', sizeof(char) * USERNAME_LENGTH);

    write(sockfd, welcome_message, strlen(welcome_message));
    
    int read_bytes = read(sockfd, buffer, BUFFER_LENGTH);
    while (read_bytes > 0 && read_bytes <= BUFFER_LENGTH) {
        printf("Command: %s\n", buffer);
        sscanf(buffer,"%s %s", command, args);
        process_command(command, args, state);
        memset(buffer, '\0', sizeof(char) * BUFFER_LENGTH);
        memset(command, '\0', sizeof(char) * COMMAND_LENGTH);
        memset(args, '\0', sizeof(char) * ARGS_LENGTH);
        read_bytes = read(sockfd, buffer, BUFFER_LENGTH);
    }

    close(sockfd);
    free(state);
    printf("Client disconnected!\n");

    return NULL;
}

void process_command(char* command, char* args, connection_state* state) {
    int cmdlist_count = sizeof(cmdlist_str) / sizeof(char *);
    int command_index = -1;
    for (int i = 0; i < cmdlist_count; i++) {
        if (strcmp(command, cmdlist_str[i]) == 0) {
            command_index = i;
            break;
        }
    }
    switch (command_index) {
        case USER: command_user(args, state); break;
        case PASS: command_pass(args, state); break;
        case RETR: command_retr(args, state); break;
        case STOR: command_stor(args, state); break;
        case QUIT: command_quit(args, state); break;
        case SYST: command_syst(args, state); break;
        case TYPE: command_type(args, state); break;
        case PORT: command_port(args, state); break;
        case PASV: command_pasv(args, state); break;
        case MKD: command_mkd(args, state); break;
        case CWD: command_cwd(args, state); break;
        case PWD: command_pwd(args, state); break;
        case LIST: command_list(args, state); break;
        case RMD: command_rmd(args, state); break;
        case RNFR: command_rnfr(args, state); break;
        case RNTO: command_rnto(args, state); break;
        default: {
            write(state->sockfd, unknown_command_message, sizeof(unknown_command_message));
            break;
        }
    }
}

void command_user(char* args, connection_state* state) {
    if (strcmp(args, username) == 0) {
        strcpy(state->username, username);
        write(state->sockfd, need_password_message, sizeof(need_password_message));
    }
    else {
        write(state->sockfd, user_invaild_message, sizeof(user_invaild_message));
    }
}

void command_pass(char* args, connection_state* state) {
    if (strcmp(state->username, username) != 0) {
        write(state->sockfd, need_login_message, sizeof(need_login_message));
        return;
    }
    if (strcmp(args, password) == 0) {
        write(state->sockfd, login_succeed_message, sizeof(login_succeed_message));
        state->logged = 1;
    }
    else {
        write(state->sockfd, wrong_password_message, sizeof(wrong_password_message));
    }
}

void command_retr(char* args, connection_state* state) {
    // TODO
}

void command_stor(char* args, connection_state* state) {
    // TODO
}

void command_quit(char* args, connection_state* state) {
    // TODO
}

void command_syst(char* args, connection_state* state) {
    // TODO
}

void command_type(char* args, connection_state* state) {
    // TODO
}

void command_port(char* args, connection_state* state) {
    if (state->logged == 0) {
        write(state->sockfd, need_login_message, sizeof(need_login_message));
        return;
    }
}

void generate_random_port(int* port1, int* port2) {
    srand(time(NULL));
    *port1 = 128 + (rand() % 64);
    *port2 = rand() % 0xff;
}

void command_pasv(char* args, connection_state* state) {
    if (state->logged == 0) {
        write(state->sockfd, need_login_message, sizeof(need_login_message));
        return;
    }
    
}

void command_mkd(char* args, connection_state* state) {
    // TODO
}

void command_cwd(char* args, connection_state* state) {
    // TODO
}

void command_pwd(char* args, connection_state* state) {
    // TODO
}

void command_list(char* args, connection_state* state) {
    // TODO
}

void command_rmd(char* args, connection_state* state) {
    // TODO
}

void command_rnfr(char* args, connection_state* state) {
    // TODO
}

void command_rnto(char* args, connection_state* state) {
    // TODO
}