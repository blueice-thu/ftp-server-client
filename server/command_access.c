#include "command_access.h"

void command_user(char* args, Session* state) {
    if (state->is_logged == 1) {
        send_message(state, "230 Already logged-in.\r\n");
        return;
    }
    state->user_index = search_username(args);
    if (state->user_index == -1) {
        send_message(state, "530 Invalid username.\r\n");
    }
    else {
        send_message(state, "331 Guest login okay, send your complete e-mail address as password.\r\n");
    }
}

void command_pass(char* args, Session* state) {
    if (state->is_logged == 1) {
        send_message(state, "202 Already logged in.\r\n");
        return;
    }
    if (state->user_index == -1) {
        send_message(state, "503 Login with USER first.\r\n");
        return;
    }
    else if (state->user_index == 0) {
        send_message(state, "230 User logged in, proceed.\r\n");
        state->is_logged = 1;
    }
    else if (check_password(state->user_index, args)) {
        send_message(state, "230 User logged in, proceed.\r\n");
        state->is_logged = 1;
    }
    else {
        send_message(state, "530 Authentication failed.\r\n");
    }
}

void command_quit(char* args, Session* state) {
    char quit_msg[] = "221-You have transferred %d bytes in %d files.\r\n"\
                "221-Total traffic for this session was %d bytes in %d transfers.\r\n"\
                "221-Thank you for using the FTP service on ftp.ssast.org.\r\n"\
                "221 Goodbye.\r\n";
    char msg[MSG_LENGTH] = { '\0' };
    sprintf(msg, quit_msg, state->trans_file_bytes, state->trans_file_num, state->trans_all_bytes, state->trans_all_num);
    send_message(state, msg);
    if (state->is_trans_data) close_trans_conn(state);
    close(state->sockfd);
}

void command_syst(char* args, Session* state) {
    send_message(state, "215 UNIX Type: L8\r\n");
}

void command_abor(char* args, Session* state) {
    command_quit(args, state);
}