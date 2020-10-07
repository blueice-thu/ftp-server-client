#include "command_trans.h"

void command_retr(char* args, Session* state) {
    if (state->logged == 0) {
        send_message(state, need_login_msg);
        return;
    }
    int sockfd = 0;
    if (state->mode == NORMAL) {
        send_message(state, "425 Use PORT or PASV first.\n");
        return;
    }
    else if (state->mode == ACTIVE) {
        // TODO
    }
    else if (state->mode == PASSIVE) {
        // TODO
    }
    else {
        printf("Wrong: mode!\n");
        exit(EXIT_FAILURE);
    }

    // if (state->mode != PASV) {
    //     char msg[] = "550 Please Use Passive Mode.\n";
    //     write(state->sockfd, msg, sizeof(msg));
    //     return;
    // }
    // struct stat file_info;
    // int send_file_bytes = 0;
    // int file_desc = open(args, O_RDONLY);
    // // TODO: process error
    // if (access(args, R_OK) && file_desc != -1) {
    //     write(state->sockfd, open_data_conn_msg, sizeof(open_data_conn_msg));
    //     fstat(file_desc, &file_info);
    //     struct sockaddr_in client_address;
    //     int address_length = sizeof(struct sockaddr_in);
    //     int connection = accept(state->passive_socket, (struct sockaddr*) &client_address, &address_length);
    //     if (connection != -1) {
    //         off_t offset = 0;
    //         send_file_bytes = sendfile(connection, file_desc, &offset, file_info.st_size);
    //         if (send_file_bytes != -1 && send_file_bytes == file_info.st_size) {
    //             write(state->sockfd, send_file_ok_msg, sizeof(send_file_ok_msg));
    //         }
    //         else {
    //             write(state->sockfd, network_fail_msg, sizeof(network_fail_msg));
    //         }
    //     }
    //     else {
    //         write(state->sockfd, fail_tcp_conn_msg, sizeof(fail_tcp_conn_msg));
    //     }
    //     close(file_desc);
    //     close(connection);
    // }
    // else {
    //     write(state->sockfd, fail_read_file_msg, sizeof(fail_read_file_msg));
    //     return;
    // }
}

void command_stor(char* args, Session* state) {
    // TODO
}