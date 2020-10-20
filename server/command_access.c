#include "command_access.h"

void command_user(char* args, Session* state) {
    if (state->is_logged == 1) {
        send_message(state, "230 Already logged-in.\n");
        return;
    }
    state->user_index = search_username(args);
    if (state->user_index == -1) {
        send_message(state, "530 Invalid username.\n");
    }
    else {
        send_message(state, "331 Guest login okay, send your complete e-mail address as password.\n");
    }
}

void command_pass(char* args, Session* state) {
    if (state->is_logged == 1) {
        send_message(state, "202 Already logged in.\n");
        return;
    }
    if (state->user_index == -1) {
        send_message(state, "503 Login with USER first.\n");
        return;
    }
    if (check_password(state->user_index, args)) {
        send_message(state, "230 User logged in, proceed.\n");
        state->is_logged = 1;
    }
    else {
        send_message(state, "530 Authentication failed.\n");
    }
}

void command_quit(char* args, Session* state) {
    char quit_msg[] = "221-You have transferred %d bytes in %d files.\n"\
                "221-Total traffic for this session was %d bytes in %d transfers.\n"\
                "221-Thank you for using the FTP service on ftp.ssast.org.\n"\
                "221 Goodbye.\n";
    char msg[MSG_LENGTH] = { '\0' };
    sprintf(msg, quit_msg, state->trans_file_bytes, state->trans_file_num, state->trans_all_bytes, state->trans_all_num);
    send_message(state, msg);
    state->is_logged = 0;
    close(state->sockfd);
}

void command_syst(char* args, Session* state) {
    send_message(state, "215 UNIX Type: L8.\n");
}

void command_abor(char* args, Session* state) {
    command_quit(args, state);
}