#include "command_access.h"

void command_user(char* args, Session* state) {
    // TODO
    strcpy(state->username, username);
    send_message(state, "331 Guest login okay, send your complete e-mail address as password.\n");
    return;

    if (state->logged == 1) {
        send_message(state, "230 Already logged-in.\n");
    }
    else if (strcmp(args, username) != 0) {
        send_message(state, "530 Invalid username.\n");
    }
    else {
        strcpy(state->username, username);
        send_message(state, "331 Guest login okay, send your complete e-mail address as password.\n");
    }
}

void command_pass(char* args, Session* state) {
    // TODO
    send_message(state, login_succeed_msg);
    state->logged = 1;
    return;

    if (state->logged == 1) {
        send_message(state, "202 Already logged in.\n");
    }
    else if (strcmp(state->username, username) != 0) {
        send_message(state, "503 Login with USER first.\n");
    }
    else if (strcmp(args, password) != 0) {
        send_message(state, "530 Authentication failed.\n");
    }
    else {
        send_message(state, "230 User logged in, proceed.\n");
        state->logged = 1;
    }
}

void command_quit(char* args, Session* state) {
    //TODO
    char quit_msg[] = "221-You have transferred %d bytes in %d files.\n"\
                "221-Total traffic for this session was %d bytes in %d transfers.\n"\
                "221-Thank you for using the FTP service on ftp.ssast.org.\n"\
                "221 Goodbye.\n";
    char msg[MSG_LENGTH] = { '\0' };
    sprintf(msg, quit_msg, state->trans_file_bytes, state->trans_file_num, state->trans_all_bytes, state->trans_all_num);
    send_message(state, msg);
}

void command_syst(char* args, Session* state) {
    //TODO
    send_message(state, "215 UNIX Type: L8.\n");
}

void command_abor(char* args, Session* state) {
    command_quit(args, state);
}