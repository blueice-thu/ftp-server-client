#include "command_access.h"

void command_user(char* args, Session* state) {
    char msg[MSG_LENGTH] = { '\0' };
    if (state->logged == 1) {
        strcpy(msg, "230 Already logged-in.\n");
    }
    else if (strcmp(args, username) != 0) {
        strcpy(msg, "530 Invalid username.\n");
    }
    else {
        strcpy(state->username, username);
        strcpy(msg, "331 Guest login okay, send your complete e-mail address as password.\n");
    }
    write(state->sockfd, msg, sizeof(msg));
}

void command_pass(char* args, Session* state) {
    // TODO
    write(state->sockfd, login_succeed_msg, sizeof(login_succeed_msg));
    state->logged = 1;
    return;

    char msg[MSG_LENGTH] = { '\0' };
    if (state->logged == 1) {
        strcpy(msg, "202 Already logged in.\n");
    }
    else if (strcmp(state->username, username) != 0) {
        strcpy(msg, "503 Login with USER first.\n");
    }
    else if (strcmp(args, password) != 0) {
        strcpy(msg, "530 Authentication failed.\n");
    }
    else {
        strcpy(msg, "230 User logged in, proceed.\n");
        state->logged = 1;
    }
    write(state->sockfd, msg, sizeof(msg));
}

void command_quit(char* args, Session* state) {
    //TODO
    char msg[] = "221-You have transferred %d bytes in %d files.\n"\
                "221-Total traffic for this session was %d bytes in %d transfers.\n"\
                "221-Thank you for using the FTP service on ftp.ssast.org.\n"\
                "221 Goodbye.\n";
    write(state->sockfd, msg, sizeof(msg));
}

void command_syst(char* args, Session* state) {
    char msg[] = "215 UNIX Type: L8.\n";
    write(state->sockfd, msg, sizeof(msg));
}

void command_abor(char* args, Session* state) {
    command_quit(args, state);
}