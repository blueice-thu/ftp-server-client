#include "netutils.h"

void receive_request(int listen_fd) {
    SockAddrIn client_address;
    socklen_t length = sizeof(client_address);
    while(1) {
        int* client_descriptor = (int*)malloc(sizeof(int));
        *client_descriptor = accept(listen_fd, (SockAddr*)&client_address, &length);

        pthread_t pid;
        pthread_create(&pid, NULL, (void*)process_request, (void*) (client_descriptor));
    }
}

void* process_request(void* client_descriptor) {
    char command[COMMAND_LENGTH] = {'\0'};
    char args[ARGS_LENGTH] = {'\0'};
    char buffer[BUFFER_LENGTH] = {'\0'};

    int sockfd = *(int*)client_descriptor;
    Session* state = (Session*)calloc(1, sizeof(Session));
    state->user_index = -1;
    state->sockfd = sockfd;
    strcpy(state->work_dir, "/");

    send_message(state, "220 Anonymous FTP server ready.\r\n");
    
    int read_bytes = read(sockfd, buffer, BUFFER_LENGTH);
    while (read_bytes > 0 && read_bytes <= BUFFER_LENGTH) {
        log_record_string(buffer);
        sscanf(buffer,"%s %s", command, args);
        process_command(command, args, state);
        memset(buffer, '\0', sizeof(char) * BUFFER_LENGTH);
        memset(command, '\0', sizeof(char) * COMMAND_LENGTH);
        memset(args, '\0', sizeof(char) * ARGS_LENGTH);
        read_bytes = read(sockfd, buffer, BUFFER_LENGTH);
    }

    close(sockfd);
    if (state->pasv_addr) free(state->pasv_addr);
    free(state);
    log_record_string("Client disconnected");

    return NULL;
}

void process_command(char* command, char* args, Session* state) {
    int command_index = -1;
    for (int i = 0; i < SUPPORTED_CMD_COUNT; i++) {
        if (strcmp(command, cmdlistStr[i]) == 0) {
            command_index = i;
            break;
        }
    }
    if (state->is_trans_data == 1 && command_index != ABOR && command_index != QUIT) {
        send_message(state, "421 Data transfering and refuse control command.\r\n");
        return;
    }
    if (state->rename_state == 1 && command_index != RNTO && command_index != ABOR && command_index != QUIT) {
        send_message(state, "421 Rename command has not been done.\r\n");
        return;
    }
    switch (command_index) {
        case USER:  command_user(args, state);  break;
        case PASS:  command_pass(args, state);  break;
        case RETR:  command_retr(args, state);  break;
        case STOR:  command_stor(args, state);  break;
        case QUIT:  command_quit(args, state);  break;
        case SYST:  command_syst(args, state);  break;
        case TYPE:  command_type(args, state);  break;
        case PORT:  command_port(args, state);  break;
        case PASV:  command_pasv(args, state);  break;
        case MKD:   command_mkd (args, state);  break;
        case CWD:   command_cwd (args, state);  break;
        case PWD:   command_pwd (args, state);  break;
        case LIST:  command_list(args, state);  break;
        case RMD:   command_rmd (args, state);  break;
        case RNFR:  command_rnfr(args, state);  break;
        case RNTO:  command_rnto(args, state);  break;
        case ABOR:  command_abor(args, state);  break;
        case DELE:  command_dele(args, state);  break;
        case CDUP:  command_cdup(args, state);  break;
        case REST:  command_rest(args, state);  break;
        default: {
            send_message(state, "500 Unknown command.\r\n");
            break;
        }
    }
}
