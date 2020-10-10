#include "common.h"

void command_mkd(char* args, Session* state) {
    if (state->logged == 0) {
        write(state->sockfd, need_login_msg, sizeof(need_login_msg));
        return;
    }
    char msg[MSG_LENGTH] = { '\0' };
    if (access(args, F_OK) == 0) {
        strcpy(msg, "550 Folder already exists.\n");
    }
    else if (mkdir(args, 0777) == -1) {
        strcpy(msg, "550 Fail to create directory.\n");
    }
    else {
        strcpy(msg, "257 Create directory \"");
        strcat(msg, args);
        strcat(msg, "\" successfully.\n");
    }
    write(state->sockfd, msg, sizeof(msg));
}

void command_cwd(char* args, Session* state) {
    if (state->logged == 0) {
        write(state->sockfd, need_login_msg, sizeof(need_login_msg));
        return;
    }
    char msg[MSG_LENGTH] = { '\0' };
    if(chdir(args) == -1) {
        strcpy(msg, "550 No such directory.\n");
    }
    else {
        strcpy(msg, "250 Change directory successfully.\n");
    }
    write(state->sockfd, msg, sizeof(msg));
}

void command_pwd(char* args, Session* state) {
    if (state->logged == 0) {
        write(state->sockfd, need_login_msg, sizeof(need_login_msg));
        return;
    }
    char msg[MSG_LENGTH] = { '\0' };
    char word_path[PATH_LENGTH] = { '\0' };
    if (getcwd(word_path, PATH_LENGTH) != NULL) {
        strcat(msg, "257 \"");
        strcat(msg, word_path);
        strcat(msg, "\"\n");
    }
    else {
        strcpy(msg, "550 Failed to get pwd.\r\n");
    }
    write(state->sockfd, msg, sizeof(msg));
}

void command_list(char* args, Session* state) {
    // TODO
    if (state->logged == 0) {
        write(state->sockfd, need_login_msg, sizeof(need_login_msg));
        return;
    }
    if (state->mode == NORMAL) {
        char msg[] = "425 Need PORT or PASV mode.\n";
        write(state->sockfd, msg, sizeof(msg));
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
        write(state->sockfd, need_login_msg, sizeof(need_login_msg));
        return;
    }
    struct stat st;
    stat(args, &st);
    char msg[MSG_LENGTH] = { '\0' };
    if (access(args, F_OK) == -1 || !S_ISDIR(st.st_mode)) {
        strcpy(msg, "550 Not a valid directory.\n");
    }
    else if (nftw(args, rmFiles, 10, FTW_DEPTH | FTW_MOUNT | FTW_PHYS) < 0) {
        perror("ERROR: ntfw");
        exit(1);
    }
    else {
        strcpy(msg, "250 Directory removed.\n");
    }
    write(state->sockfd, msg, sizeof(msg));
}

void command_rnfr(char* args, Session* state) {
    if (state->logged == 0) {
        write(state->sockfd, need_login_msg, sizeof(need_login_msg));
        return;
    }
    if (access(args, R_OK) == -1) {
        write(state->sockfd, no_file_msg, sizeof(no_file_msg));
        return;
    }
    if (access(args, W_OK) == -1) {
        write(state->sockfd, no_permis_msg, sizeof(no_permis_msg));
        return;
    }
    if (state->rename_from) 
        free(state->rename_from);
    state->rename_from = (char*)malloc(PATH_LENGTH);
    strcpy(state->rename_from, args);

    char msg[] = "350 Ready to rename file.\n";
    write(state->sockfd, msg, sizeof(msg));
}

void command_rnto(char* args, Session* state) {
    if (state->logged == 0) {
        write(state->sockfd, need_login_msg, sizeof(need_login_msg));
        return;
    }
    char msg[MSG_LENGTH] = { '\0' };
    if (state->rename_from == NULL) {
        strcpy(msg, "550 No file is specified.\n");
    }
    else if (rename(state->rename_from, args) == -1) {
        strcpy(msg, "550 Fail to rename file.\n");
    }
    else 
        strcpy(msg, "250 Rename file successfully.\n");
    free(state->rename_from); state->rename_from = NULL;
    write(state->sockfd, msg, sizeof(msg));
}

void command_dele(char* args, Session* state) {
    if (state->logged == 0) {
        write(state->sockfd, need_login_msg, sizeof(need_login_msg));
        return;
    }
    char msg[MSG_LENGTH] = { '\0' };
    if (unlink(args) == 0) {
        strcpy(msg, "250 Delete file successfully.\n");
    }
    else {
        strcpy(msg, "550 Fail to delete file.\n");
    }
    write(state->sockfd, msg, sizeof(msg));
}

void command_cdup(char* args, Session* state) {
    if (state->logged == 0) {
        write(state->sockfd, need_login_msg, sizeof(need_login_msg));
        return;
    }
    char msg[MSG_LENGTH] = { '\0' };
    if(chdir("..") == -1) {
        strcpy(msg, "550 Fail to change directory.\n");
    }
    else {
        strcpy(msg, "250 Change directory successfully.\n");
    }
    write(state->sockfd, msg, sizeof(msg));
}