#include "command_trans.h"

typedef struct param_buffer {
    FILE* fp;
    Session* state;
}param_buffer;

void _command_retr(void* paras) {
    param_buffer* para = (param_buffer*)paras;
    FILE* fp = para->fp;
    Session* state = para->state;

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

void command_retr(char* args, Session* state) {
    if (state->is_logged == 0) {
        send_message(state, need_login_msg);
        return;
    }
    char full_args_path[PATH_LENGTH] = { '\0' };
    int join_status = get_args_full_path(state, args, full_args_path);

    if (join_status == 0 || access(full_args_path, F_OK) == -1) {
        send_message(state, "550 No such file or directory.\r\n");
        return ;
    }

    FILE* fp = fopen(full_args_path, "rb");
    fseek(fp, state->rest_pos, SEEK_SET);
    state->rest_pos = 0;
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

    pthread_t pid;
    param_buffer* paras = (param_buffer*)malloc(sizeof(param_buffer));
    paras->fp = fp;
    paras->state = state;
    pthread_create(&pid, NULL, (void*)_command_retr, (void*)paras);
}

void _command_stor(void* paras) {
    param_buffer* para = (param_buffer*)paras;
    FILE* fp = para->fp;
    Session* state = para->state;

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

void command_stor(char* args, Session* state) {
    if (state->is_logged == 0) {
        send_message(state, need_login_msg);
        return;
    }
    char full_args_path[PATH_LENGTH] = { '\0' };
    int join_status = get_args_full_path(state, args, full_args_path);

    FILE* fp = NULL;
    if (state->rest_pos <= 0)
        fp = fopen(full_args_path, "wb");
    else {
        fp = fopen(full_args_path, "wb+");
        fseek(fp, state->rest_pos, SEEK_SET);
        state->rest_pos = 0;
    }
    
    if (join_status == 0 || fp == NULL) {
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

    pthread_t pid;
    param_buffer* paras = (param_buffer*)malloc(sizeof(param_buffer));
    paras->fp = fp;
    paras->state = state;
    pthread_create(&pid, NULL, (void*)_command_stor, (void*)paras);
}

void command_rest(char* args, Session* state) {
    if (state->is_logged == 0) {
        send_message(state, need_login_msg);
        return;
    }
    int pos = atoi(args);
    if (pos < 0) {
        send_message(state, "501 Marker cannot be negetive.\r\n");
    }
    else if (pos == 0) {
        send_message(state, "501 Not a valid number.\r\n");
    }
    else {
        state->rest_pos = pos;
        char msg[MSG_LENGTH];
        sprintf(msg, "Restarting at %d. Send STORE or RETRIEVE to initiate transfer.\r\n", pos);
        send_message(state, msg);
    }
}