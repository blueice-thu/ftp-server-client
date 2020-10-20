#include "command_trans.h"

void command_retr(char* args, Session* state) {
    if (state->is_logged == 0) {
        send_message(state, need_login_msg);
        return;
    }
    if (strstr(args, "../") != NULL) {
        send_message(state, "550 Permission denied.\r\n");
        return ;
    }
    if (access(args, F_OK) != 0) {
        send_message(state, "550 No such file or directory.\r\n");
        return ;
    }

    FILE* fp = fopen(args, "rb");
    if (fp == NULL) {
        send_message(state, "551 Permission denied.\r\n");
        return ;
    }

    if (state->mode == NORMAL) {
        send_message(state, "425 Use PORT or PASV first.\r\n");
        return;
    }
    update_data_trans_fd(state);
    if (state->data_trans_fd <= 0) {
        send_message(state, "425 Fail to establish connection.\r\n");
        return;
    }
    send_message(state, "150 Opening data connection.\r\n");

    state->is_trans_data = 1;

    char buffer[BUFFER_LENGTH] = { 0 };
    while (!feof(fp)) {
        int read_bytes = fread(buffer, sizeof(char), BUFFER_LENGTH - 1, fp);
        int send_bytes = send(state->data_trans_fd, buffer, read_bytes, 0);
        state->trans_file_bytes += send_bytes;
        state->trans_all_bytes += send_bytes;
        memset(buffer, 0, BUFFER_LENGTH);
    }

    state->is_trans_data = 0;
    fclose(fp); close_trans_conn(state);

    send_message(state, "226 Transfer complete.\r\n");
    state->trans_file_num += 1;
}

void command_stor(char* args, Session* state) {
    if (state->is_logged == 0) {
        send_message(state, need_login_msg);
        return;
    }
    if (strstr(args, "../") != NULL) {
        send_message(state, "550 Permission denied.\r\n");
        return ;
    }
    
    FILE* fp = fopen(args, "wb");
    if (fp == NULL) {
        send_message(state, "551 Permission denied.\r\n");
        return ;
    }

    if (state->mode == NORMAL) {
        send_message(state, "425 Use PORT or PASV first.\r\n");
        return;
    }
    update_data_trans_fd(state);
    if (state->data_trans_fd <= 0) {
        send_message(state, "425 Fail to establish connection.\r\n");
        return;
    }
    send_message(state, "150 Opening data connection.\r\n");

    char buffer[BUFFER_LENGTH] = { 0 };
    int recv_length = 0, write_length = 0;
    state->is_trans_data = 1;
    recv_length = recv(state->data_trans_fd, buffer, BUFFER_LENGTH, 0);
    while (recv_length) {
        if (recv_length < 0) {
            send_message(state, "426 Data connection error.\r\n");
            state->is_trans_data = 0;
            fclose(fp); close_trans_conn(state);
            return;
        }
        state->trans_file_bytes += recv_length;
        state->trans_all_bytes += recv_length;
        write_length = fwrite(buffer, sizeof(char), recv_length, fp);
        if (write_length < recv_length) {
            state->is_trans_data = 0;
            send_message(state, "426 Data connection error.\r\n");
            fclose(fp); close_trans_conn(state);
            return;
        }
        memset(buffer, 0, BUFFER_LENGTH);
        recv_length = recv(state->data_trans_fd, buffer, BUFFER_LENGTH, 0);
    }

    state->is_trans_data = 0;
    fclose(fp); close_trans_conn(state);

    send_message(state, "226 Transfer complete.\r\n");
    state->trans_file_num += 1;
}