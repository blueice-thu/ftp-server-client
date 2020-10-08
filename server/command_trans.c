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
}

void command_stor(char* args, Session* state) {
    // TODO
}