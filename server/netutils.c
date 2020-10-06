#include "netutils.h"

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
    return sockfd;
}

int create_ftp_server(const char *host, unsigned short port) {
    if(chroot(root_path) !=0 ) {
       printf("Wrong: cannot find root path!\n");
       exit(EXIT_FAILURE);
    }
    int sockfd = create_socket(port);
    if (sockfd < 0) {
        printf("Wrong: fail to create socket!\n");
        exit(EXIT_FAILURE);
    }
    if(listen(sockfd, SOMAXCONN) < 0) {
        close(sockfd);
        printf("Wrong: fail to listen!\n");
        exit(EXIT_FAILURE);
    }
    return sockfd;
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
    state->mode = NORMAL;
    state->rename_from = NULL;
    memset(state->username, '\0', sizeof(char) * USERNAME_LENGTH);

    write(sockfd, welcome_msg, strlen(welcome_msg));
    
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
        case ABOR: command_abor(args, state); break;
        case DELE: command_dele(args, state); break;
        default: {
            write(state->sockfd, unknown_command_msg, sizeof(unknown_command_msg));
            break;
        }
    }
}

void command_user(char* args, connection_state* state) {
    if (strcmp(args, username) == 0) {
        strcpy(state->username, username);
        write(state->sockfd, need_password_msg, sizeof(need_password_msg));
    }
    else {
        write(state->sockfd, user_invaild_msg, sizeof(user_invaild_msg));
    }
}

void command_pass(char* args, connection_state* state) {
    // TODO
    write(state->sockfd, login_succeed_msg, sizeof(login_succeed_msg));
    state->logged = 1;
    return;
    
    if (strcmp(state->username, username) != 0) {
        write(state->sockfd, need_login_msg, sizeof(need_login_msg));
    }
    else if (strcmp(args, password) != 0) {
        write(state->sockfd, wrong_password_msg, sizeof(wrong_password_msg));
    }
    else {
        write(state->sockfd, login_succeed_msg, sizeof(login_succeed_msg));
        state->logged = 1;
    }
}

void command_retr(char* args, connection_state* state) {
    if (state->logged == 0) {
        write(state->sockfd, need_login_msg, sizeof(need_login_msg));
        return;
    }
    if (state->mode != PASV) {
        write(state->sockfd, need_passive_msg, sizeof(need_passive_msg));
        return;
    }
    struct stat file_info;
    int send_file_bytes = 0;
    int file_desc = open(args, O_RDONLY);
    // TODO: process error
    if (access(args, R_OK) && file_desc != -1) {
        write(state->sockfd, open_data_conn_msg, sizeof(open_data_conn_msg));
        fstat(file_desc, &file_info);
        struct sockaddr_in client_address;
        int address_length = sizeof(struct sockaddr_in);
        int connection = accept(state->passive_socket, (struct sockaddr*) &client_address, &address_length);
        if (connection != -1) {
            off_t offset = 0;
            send_file_bytes = sendfile(connection, file_desc, &offset, file_info.st_size);
            if (send_file_bytes != -1 && send_file_bytes == file_info.st_size) {
                write(state->sockfd, send_file_ok_msg, sizeof(send_file_ok_msg));
            }
            else {
                write(state->sockfd, network_fail_msg, sizeof(network_fail_msg));
            }
        }
        else {
            write(state->sockfd, fail_tcp_conn_msg, sizeof(fail_tcp_conn_msg));
        }
        close(file_desc);
        close(connection);
    }
    else {
        write(state->sockfd, fail_read_file_msg, sizeof(fail_read_file_msg));
        return;
    }
}

void command_stor(char* args, connection_state* state) {
    // TODO
}

void command_quit(char* args, connection_state* state) {
    //TODO
    write(state->sockfd, quit_msg, sizeof(quit_msg));
}

void command_syst(char* args, connection_state* state) {
    write(state->sockfd, syst_msg, sizeof(syst_msg));
}

void command_type(char* args, connection_state* state) {
    if (strcmp(args, "I") == 0) {
        write(state->sockfd, type_msg, sizeof(type_msg));
    }
    else {
        write(state->sockfd, type_wrong_msg, sizeof(type_wrong_msg));
    }
}

void command_port(char* args, connection_state* state) {
    if (state->logged == 0) {
        write(state->sockfd, need_login_msg, sizeof(need_login_msg));
        return;
    }
}

// void generate_random_port(int* port1, int* port2) {
//     srand(time(NULL));
//     *port1 = 128 + (rand() % 64);
//     *port2 = rand() % 0xff;
// }

void command_pasv(char* args, connection_state* state) {
    if (state->logged == 0) {
        write(state->sockfd, need_login_msg, sizeof(need_login_msg));
        return;
    }

    int port1 = 0, port2 = 0, port;
    srand(time(NULL));
    port1 = 128 + (rand() % 64);
    port2 = rand() % 0xff;
    // generate_random_port(&port1, &port2);
    port = 256 * port1 + port2;

    int sockfd = state->sockfd;
    socklen_t addr_size = sizeof(struct sockaddr_in);
    struct sockaddr_in addr;
    if (getsockname(sockfd, (struct sockaddr *)&addr, &addr_size) != 0) {
        printf("Wrong: unknown error!\n");
        exit(EXIT_FAILURE);
    }
    int ip[4] = { 0 };
    int host = addr.sin_addr.s_addr;
    for (int i = 0; i < 4; i++) 
        ip[i] = (host >> i * 8) & 0xff;
    
    close(state->passive_socket);
    state->passive_socket = create_socket(port);
    state->mode = PASSIVE;
    printf("Passive port: %d\n", state->passive_socket);

    char message[128] = { '\0' };
    sprintf(message, passive_msg, ip[0], ip[1], ip[2], ip[3], port1, port2);
    write(state->sockfd, message, sizeof(message));
    printf("%s\n", message);
}

void command_mkd(char* args, connection_state* state) {
    // TODO
}

void command_cwd(char* args, connection_state* state) {
    if (state->logged == 0) {
        write(state->sockfd, need_login_msg, sizeof(need_login_msg));
        return;
    }
    if(chdir(args) == 0){
        write(state->sockfd, change_dir_msg, sizeof(change_dir_msg));
    }else{
        write(state->sockfd, fail_chdir_msg, sizeof(fail_chdir_msg));
    }
}

void command_pwd(char* args, connection_state* state) {
    if (state->logged == 0) {
        write(state->sockfd, need_login_msg, sizeof(need_login_msg));
        return;
    }
    char msg[MSG_LENGTH] = { '\0' };
    char word_path[PATH_LENGTH] = { '\0' };
    if (getcwd(word_path, PATH_LENGTH) != NULL) {
        strcat(msg, "257 \"");
        strcat(msg, word_path);
        strcat(msg, "\"\n");
    }
    else {
        strcpy(msg, "550 Failed to get pwd.\r\n");
    }
    write(state->sockfd, msg, sizeof(msg));
}

void command_list(char* args, connection_state* state) {
    // TODO
    if (state->logged == 0) {
        write(state->sockfd, need_login_msg, sizeof(need_login_msg));
        return;
    }
}

void command_rmd(char* args, connection_state* state) {
    // TODO
}

void command_rnfr(char* args, connection_state* state) {
    if (state->logged == 0) {
        write(state->sockfd, need_login_msg, sizeof(need_login_msg));
        return;
    }
    if (access(args, R_OK) == -1) {
        write(state->sockfd, no_file_msg, sizeof(no_file_msg));
        return;
    }
    if (access(args, W_OK) == -1) {
        write(state->sockfd, no_permis_msg, sizeof(no_permis_msg));
        return;
    }
    if (state->rename_from) 
        free(state->rename_from);
    state->rename_from = (char*)malloc(PATH_LENGTH);
    strcpy(state->rename_from, args);

    char msg[] = "350 Ready to rename file.\n";
    write(state->sockfd, msg, sizeof(msg));
}

void command_rnto(char* args, connection_state* state) {
    if (state->logged == 0) {
        write(state->sockfd, need_login_msg, sizeof(need_login_msg));
        return;
    }
    char msg[MSG_LENGTH] = { '\0' };
    if (state->rename_from == NULL) {
        strcpy(msg, "550 No file is specified.\n");
    }
    else if (rename(state->rename_from, args) == -1) {
        strcpy(msg, "550 Fail to rename file.\n");
    }
    else 
        strcpy(msg, "250 Rename file successfully.\n");
    free(state->rename_from); state->rename_from = NULL;
    write(state->sockfd, msg, sizeof(msg));
}

void command_abor(char* args, connection_state* state) {
    command_quit(args, state);
}

void command_dele(char* args, connection_state* state) {
    if (state->logged == 0) {
        write(state->sockfd, need_login_msg, sizeof(need_login_msg));
        return;
    }
    char msg[MSG_LENGTH] = { '\0' };
    if (unlink(args) == 0) {
        strcpy(msg, "250 Delete file successfully.\n");
    }
    else {
        strcpy(msg, "550 Fail to delete file.\n");
    }
    write(state->sockfd, msg, sizeof(msg));
}