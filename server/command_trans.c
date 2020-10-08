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
    else if (state->mode == ACTIVE) {
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
    else if (state->mode == PASSIVE) {
        // TODO
    }
    else {
        printf("Wrong: mode!\n");
        exit(EXIT_FAILURE);
    }
}

void command_stor(char* args, Session* state) {
    // TODO
    if (state->logged == 0) {
        send_message(state, need_login_msg);
        return;
    }
    if (state->mode == NORMAL) {
        send_message(state, "425 Use PORT or PASV first.\n");
        return;
    }
    else if (state->mode == ACTIVE) {
        FILE* fp = fopen(args, "w");
        if (fp == NULL) {
            send_message(state, "550 No such file or directory.\n");
            return ;
        }
        send_message(state, "150 Opening data connection.\n");
        int receive_bytes = 0, n = 0;
        char buffer[BUFFER_LENGTH];
        while (1) {
            n = 0;
            while (1) {
                receive_bytes = recv(state->data_trans_fd, &buffer[n], 1, 0);
                if (receive_bytes <= 0) break;
                if (buffer[n] == '\n') {
                    buffer[n] == '\0';
                    break;
                }
                if (buffer[n] == '\r') n++;
            }
            if (receive_bytes <= 0) break;
            fprintf(fp, "%s\n", buffer);
        }
        send_message(state, "226 Transfer complete.\n");
        fclose(fp);
        close(state->data_trans_fd);
    }
    else if (state->mode == PASSIVE) {
        // TODO
    }
    else {
        printf("Wrong: mode!\n");
        exit(EXIT_FAILURE);
    }
}