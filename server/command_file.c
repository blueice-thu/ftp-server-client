#include "common.h"

void command_mkd(char* args, Session* state) {
    if (state->logged == 0) {
        send_message(state, need_login_msg);
        return;
    }
    if (access(args, F_OK) == 0) {
        send_message(state, "550 Folder already exists.\n");
    }
    else if (mkdir(args, 0777) == -1) {
        send_message(state, "550 Fail to create directory.\n");
    }
    else {
        char msg[MSG_LENGTH] = { '\0' };
        strcpy(msg, "257 Create directory \"");
        strcat(msg, args);
        strcat(msg, "\" successfully.\n");
        send_message(state, msg);
    }
}

void command_cwd(char* args, Session* state) {
    if (state->logged == 0) {
        send_message(state, need_login_msg);
        return;
    }
    if(chdir(args) == -1) {
        send_message(state, "550 No such directory.\n");
    }
    else {
        send_message(state, "250 Change directory successfully.\n");
    }
}

void command_pwd(char* args, Session* state) {
    if (state->logged == 0) {
        send_message(state, need_login_msg);
        return;
    }
    
    char word_path[PATH_LENGTH] = { '\0' };
    if (getcwd(word_path, PATH_LENGTH) != NULL) {
        char msg[MSG_LENGTH] = { '\0' };
        strcat(msg, "257 \"");
        strcat(msg, word_path);
        strcat(msg, "\"\n");
        send_message(state, need_login_msg);
    }
    else {
        send_message(state, "550 Failed to get pwd.\r\n");
    }
}

void command_list(char* args, Session* state) {
    // TODO
    if (state->logged == 0) {
        send_message(state, need_login_msg);
        return;
    }
    if (state->mode == NORMAL) {
        send_message(state, "425 Need PORT or PASV mode.\n");
        return;
    }
    else if (state->mode == ACTIVE || state->mode == PASSIVE) {
        DIR* dir_ptr;
        struct dirent *direntp;
        if ((dir_ptr = opendir(".")) == NULL) {
            send_message(state, "551 File listing failed.\n");
            return ;
        }
        else {
            if (state->mode == PASSIVE) {
                state->data_trans_fd = accept(state->passive_socket, NULL, NULL);
            }
            send_message(state, "150 Opening data connection.\n");
            while((direntp = readdir(dir_ptr)) != NULL) {
                if(strcmp(direntp->d_name, ".") != 0 && strcmp(direntp->d_name, "..") != 0) {
                    strcat(direntp->d_name, " ");
                    send(state->data_trans_fd, direntp->d_name, strlen(direntp->d_name), 0);
                }
            }
            closedir(dir_ptr);
        }
        close_trans_conn(state);
        send_message(state, "226 Closing data connection.\n");
    }
    else {
        printf("Wrong: mode!\n");
        exit(EXIT_FAILURE);
    }
}

static int rmFiles(const char *pathname, const struct stat *sbuf, int type, struct FTW *ftwb)
{
    if(remove(pathname) < 0)
    {
        perror("ERROR: remove");
        return -1;
    }
    return 0;
}

void command_rmd(char* args, Session* state) {
    // TODO
    if (state->logged == 0) {
        send_message(state, need_login_msg);
        return;
    }
    struct stat st;
    stat(args, &st);
    char msg[MSG_LENGTH] = { '\0' };
    if (access(args, F_OK) == -1 || !S_ISDIR(st.st_mode)) {
        send_message(state, "550 Not a valid directory.\n");
    }
    else if (nftw(args, rmFiles, 10, FTW_DEPTH | FTW_MOUNT | FTW_PHYS) < 0) {
        perror("ERROR: ntfw");
        exit(1);
    }
    else {
        send_message(state, "250 Directory removed.\n");
    }
}

void command_rnfr(char* args, Session* state) {
    if (state->logged == 0) {
        send_message(state, need_login_msg);
        return;
    }
    if (access(args, R_OK) == -1) {
        send_message(state, no_file_msg);
        return;
    }
    if (access(args, W_OK) == -1) {
        send_message(state, no_permis_msg);
        return;
    }
    if (state->rename_from) 
        free(state->rename_from);
    state->rename_from = (char*)malloc(PATH_LENGTH);
    strcpy(state->rename_from, args);

    send_message(state, "350 Ready to rename file.\n");
}

void command_rnto(char* args, Session* state) {
    if (state->logged == 0) {
        send_message(state, need_login_msg);
        return;
    }
    if (state->rename_from == NULL) {
        send_message(state, "550 No file is specified.\n");
    }
    else if (rename(state->rename_from, args) == -1) {
        send_message(state, "550 Fail to rename file.\n");
    }
    else 
        send_message(state, "250 Rename file successfully.\n");
    free(state->rename_from); state->rename_from = NULL;
}

void command_dele(char* args, Session* state) {
    if (state->logged == 0) {
        send_message(state, need_login_msg);
        return;
    }
    if (unlink(args) == 0) {
        send_message(state, "250 Delete file successfully.\n");
    }
    else {
        send_message(state, "550 Fail to delete file.\n");
    }
}

void command_cdup(char* args, Session* state) {
    if (state->logged == 0) {
        send_message(state, need_login_msg);
        return;
    }
    if(chdir("..") == -1) {
        send_message(state, "550 Fail to change directory.\n");
    }
    else {
        send_message(state, "250 Change directory successfully.\n");
    }
}