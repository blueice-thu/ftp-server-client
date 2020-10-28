#include "common.h"

void command_mkd(char* args, Session* state) {
    if (state->is_logged == 0) {
        send_message(state, need_login_msg);
        return;
    }
    
    char real_work_dir[PATH_LENGTH] = { '\0' };
    char full_dir_path[PATH_LENGTH] = { '\0' };
    join_path(config.root_path, state->work_dir, real_work_dir);
    int join_status = join_path(real_work_dir, args, full_dir_path);

    if (join_status == 0 || is_valid_path(real_work_dir) == 0) {
        send_message(state, "550 Fail to create directory.\r\n");
        return;
    }
    if (access(full_dir_path, F_OK) == 0) {
        send_message(state, "550 Folder already exists.\r\n");
    }
    else if (mkdir(full_dir_path, 0777) == -1) {
        send_message(state, "550 Fail to create directory.\r\n");
    }
    else {
        char msg[MSG_LENGTH] = { '\0' };
        strcpy(msg, "257 Create directory \"");
        strcat(msg, args);
        strcat(msg, "\" successfully.\r\n");
        send_message(state, msg);
    }
}

void command_cwd(char* args, Session* state) {
    if (state->is_logged == 0) {
        send_message(state, need_login_msg);
        return;
    }
    char full_args_path[PATH_LENGTH] = { '\0' };
    int join_status = get_args_full_path(state, args, full_args_path);

    struct stat st;
    stat(full_args_path, &st);

    if (join_status == 0 || access(full_args_path, F_OK) == -1 || !S_ISDIR(st.st_mode)) {
        send_message(state, "550 No such directory.\r\n");
    }
    else if (is_valid_path(full_args_path) == 0) {
        send_message(state, no_permis_msg);
    }
    else {
        char new_work_dir[PATH_LENGTH] = { '\0' };
        join_path(state->work_dir, args, new_work_dir);
        strcpy(state->work_dir, new_work_dir);
        send_message(state, "250 Command okay.\r\n");
    }
}

void command_pwd(char* args, Session* state) {
    if (state->is_logged == 0) {
        send_message(state, need_login_msg);
        return;
    }
    char msg[MSG_LENGTH] = { '\0' };
    strcat(msg, "257 \"");
    strcat(msg, state->work_dir);
    strcat(msg, "\"\r\n");
    send_message(state, msg);
}

void command_list(char* args, Session* state) {
    if (state->is_logged == 0) {
        send_message(state, need_login_msg);
        return;
    }

    char real_dir[PATH_LENGTH] = { '\0' };
    int join_status = join_path(config.root_path, state->work_dir, real_dir);

    if (join_status == 0) {
        send_message(state, "551 File listing failed.\r\n");
    }

    if (real_dir[strlen(real_dir - 1)] != '/')
        strcat(real_dir, "/");

    chdir(real_dir);
    DIR* dir_ptr = opendir(".");
    if (dir_ptr == NULL) {
        send_message(state, "551 File listing failed.\r\n");
        return ;
    }

    update_data_trans_fd(state);
    if (state->mode == NORMAL) {
        send_message(state, "425 Use PORT or PASV first.\r\n");
        return;
    }
    if (state->data_trans_fd <= 0) {
        send_message(state, "425 Fail to establish connection.\r\n");
        return;
    }
    send_message(state, "150 Opening data connection.\r\n");

    struct dirent *dr;
    while((dr = readdir(dir_ptr)))
    {
        const char *filename = dr->d_name;
        if(filename[0] == '.')
            continue;

        struct stat data_info;
        if(lstat(filename, &data_info) == -1) {
            log_record_string("Error: LIST");
            exit(EXIT_FAILURE);
        }
        char buffer[BUFFER_LENGTH] = "%s";
        strncpy(buffer, filename, BUFFER_LENGTH);
        strcat(buffer, "\r\r\n");
        
        int bytes = write(state->data_trans_fd, buffer, strlen(buffer));
        state->trans_all_bytes += bytes;
    }
    state->trans_all_num += 1;
    closedir(dir_ptr);
    chdir(config.root_path);
        
    close_trans_conn(state);
    send_message(state, "226 Closing data connection.\r\n");
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
    if (state->is_logged == 0) {
        send_message(state, need_login_msg);
        return;
    }

    char full_args_path[PATH_LENGTH] = { '\0' };
    int join_status = get_args_full_path(state, args, full_args_path);

    struct stat st;
    stat(full_args_path, &st);

    if (join_status == 0 || access(full_args_path, F_OK) == -1 || !S_ISDIR(st.st_mode)) {
        send_message(state, "550 Not a valid directory.\r\n");
    }
    else if (nftw(full_args_path, rmFiles, 10, FTW_DEPTH | FTW_MOUNT | FTW_PHYS) < 0) {
        perror("ERROR: ntfw");
        exit(1);
    }
    else {
        send_message(state, "250 Directory removed.\r\n");
    }
}

void command_rnfr(char* args, Session* state) {
    if (state->is_logged == 0) {
        send_message(state, need_login_msg);
        return;
    }

    char full_args_path[PATH_LENGTH] = { '\0' };
    int join_status = get_args_full_path(state, args, full_args_path);

    if (join_status == 0 || access(full_args_path, R_OK) == -1) {
        send_message(state, no_file_msg);
    }
    else {
        state->rename_state = 1;
        strcpy(state->rename_from, full_args_path);
        send_message(state, "350 Ready to rename file.\r\n");
    }
}

void command_rnto(char* args, Session* state) {
    if (state->is_logged == 0) {
        send_message(state, need_login_msg);
        return;
    }
    if (strlen(state->rename_from) == 0 || state->rename_state == 0) {
        send_message(state, "550 No file is specified.\r\n");
        return;
    }

    char full_args_path[PATH_LENGTH] = { '\0' };
    int join_status = get_args_full_path(state, args, full_args_path);

    if (join_status == 0 || rename(state->rename_from, full_args_path) == -1) {
        send_message(state, "550 Fail to rename file.\r\n");
    }
    else {
        send_message(state, "250 Rename file successfully.\r\n");
    }

    state->rename_state = 0;
}

void command_dele(char* args, Session* state) {
    if (state->is_logged == 0) {
        send_message(state, need_login_msg);
        return;
    }
    char full_args_path[PATH_LENGTH] = { '\0' };
    int join_status = get_args_full_path(state, args, full_args_path);
    
    if (join_status == 0 || unlink(full_args_path) == -1) {
        send_message(state, "550 Fail to delete file.\r\n");
    }
    else {
        send_message(state, "250 Delete file successfully.\r\n");
    }
}

void command_cdup(char* args, Session* state) {
    if (state->is_logged == 0) {
        send_message(state, need_login_msg);
        return;
    }
    command_cwd("..", state);
}