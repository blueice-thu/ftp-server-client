#include "command_trans.h"

void command_retr(char* args, Session* state) {
    if (state->is_logged == 0) {
        send_message(state, need_login_msg);
        return;
    }
    printf("command_retr begin\n");
    if (state->mode == NORMAL || state->data_trans_fd <= 0) {
        send_message(state, "425 Use PORT or PASV first.\n");
        return;
    }
    FILE* fp = fopen(args, "r");
    if (fp == NULL) {
        send_message(state, "550 No such file or directory.\n");
        return ;
    }
    send_message(state, "150 Opening data connection.\n");

    char buffer[BUFFER_LENGTH] = { 0 };
    while (!feof(fp)) {
        fgets(buffer, BUFFER_LENGTH, fp);
        int bytes = send(state->data_trans_fd, buffer, strlen(buffer), 0);
        state->trans_file_bytes += bytes;
        state->trans_all_bytes += bytes;
        memset(buffer, 0, BUFFER_LENGTH);
    }
    fclose(fp);
    send_message(state, "226 Transfer complete.\n");
    state->trans_file_num += 1;
    
    close_trans_conn(state);
    printf("command_retr end\n");
}

void command_stor(char* args, Session* state) {
    if (state->is_logged == 0) {
        send_message(state, need_login_msg);
        return;
    }
    if (state->mode == NORMAL) {
        send_message(state, "425 Use PORT or PASV first.\n");
        return;
    }
    FILE* fp = fopen(args, "w");
    if (fp == NULL) {
        send_message(state, "550 No such file or directory.\n");
        return ;
    }
    send_message(state, "150 Opening data connection.\n");

    char buffer[BUFFER_LENGTH] = { 0 };
    int recv_length = 0, write_length = 0;
    while (recv_length = recv(state->data_trans_fd, buffer, BUFFER_LENGTH, 0)) {
        if (recv_length < 0) {
            send_message(state, "426 Data connection error.\n");
            fclose(fp);
            close_trans_conn(state);
            break;
        }
        state->trans_file_bytes += recv_length;
        state->trans_all_bytes += recv_length;
        write_length = fwrite(buffer, sizeof(char), recv_length, fp);
        if (write_length < recv_length) {
            send_message(state, "551 Error on output file.\n");
            fclose(fp);
            close_trans_conn(state);
            break;
        }
        memset(buffer, 0, BUFFER_LENGTH);
    }
    send_message(state, "226 Transfer complete.\n");
    state->trans_file_num += 1;
    fclose(fp);
    close_trans_conn(state);
}