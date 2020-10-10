#include "command_trans.h"

void command_retr(char* args, Session* state) {
    if (state->logged == 0) {
        send_message(state, need_login_msg);
        return;
    }
    if (state->mode == NORMAL) {
        send_message(state, "425 Use PORT or PASV first.\n");
        return;
    }
    else if (state->mode == ACTIVE || state->mode == PASSIVE) {
        if (state->mode == PASSIVE) {
            if (state->data_trans_fd > 2)
                close(state->data_trans_fd);
            struct sockaddr_in client_address;
            int addrlen = sizeof(client_address);
            state->data_trans_fd = accept(state->passive_socket, (struct sockaddr*) &client_address, &addrlen);
            close(state->passive_socket);
        }
        FILE* fp = fopen(args, "r");
        if (fp == NULL) {
            send_message(state, "550 No such file or directory.\n");
            return ;
        }
        else {
            send_message(state, "150 Opening data connection.\n");
            char buffer[BUFFER_LENGTH];
            while (!feof(fp)) {
                fgets(buffer, DATA_BUFFER, fp);
                send(state->data_trans_fd, buffer, strlen(buffer), 0);
            }
            fclose(fp);
            send_message(state, "226 Transfer complete.\n");
        }
        close(state->data_trans_fd);
    }
    else {
        printf("Wrong: mode!\n");
        exit(EXIT_FAILURE);
    }
}

void command_stor(char* args, Session* state) {
    if (state->logged == 0) {
        send_message(state, need_login_msg);
        return;
    }
    if (state->mode == NORMAL) {
        send_message(state, "425 Use PORT or PASV first.\n");
        return;
    }
    else if (state->mode == ACTIVE || state->mode == PASSIVE) {
        if (state->mode == PASSIVE) {
            if (state->data_trans_fd > 2)
                close(state->data_trans_fd);
            struct sockaddr_in client_address;
            int addrlen = sizeof(client_address);
            state->data_trans_fd = accept(state->passive_socket, (struct sockaddr*) &client_address, &addrlen);
            close(state->passive_socket);
        }
        FILE* fp = fopen(args, "w");
        if (fp == NULL) {
            send_message(state, "550 No such file or directory.\n");
            return ;
        }
        send_message(state, "150 Opening data connection.\n");
        int file_handle = fileno(fp);
        int pipefd[2];
        int res = 1;
        if(pipe(pipefd)==-1) perror("ftp_stor: pipe");
        while ((res = splice(state->data_trans_fd, 0, pipefd[1], NULL, BUFFER_LENGTH, SPLICE_F_MORE | SPLICE_F_MOVE))>0){
            splice(pipefd[0], NULL, file_handle, 0, BUFFER_LENGTH, SPLICE_F_MORE | SPLICE_F_MOVE);
        }
        send_message(state, "226 Transfer complete.\n");
        close(file_handle);
        fclose(fp);
        close(state->data_trans_fd);
    }
    else {
        printf("Wrong: mode!\n");
        exit(EXIT_FAILURE);
    }
}