#include "netutils.h"

int create_socket(int port)
{
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        printf("Wrong: fail to open socket!\n");
        return -1; 
    }

    struct sockaddr_in sock_addr;
    sock_addr.sin_family = AF_INET;
    sock_addr.sin_port = htons(port);
    sock_addr.sin_addr.s_addr = htonl(INADDR_ANY);        

    int reuse = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(int)) == -1) {
        close(sockfd);
        printf("Wrong: fail to set sockopt!\n");
        return -1; 
    }

    if (bind(sockfd, (struct sockaddr *) &sock_addr, sizeof(sock_addr)) < 0) {
        close(sockfd);
        printf("Wrong: fail to bind!\n");
        return -1; 
    }     
    return sockfd;
}

int create_ftp_server(const char *host, unsigned short port) {
    if(chroot(root_path) !=0 ) {
       printf("Wrong: cannot find root path!\n");
       exit(EXIT_FAILURE);
    }
    int sockfd = create_socket(port);
    if (sockfd < 0) {
        printf("Wrong: fail to create socket!\n");
        exit(EXIT_FAILURE);
    }
    if(listen(sockfd, SOMAXCONN) < 0) {
        close(sockfd);
        printf("Wrong: fail to listen!\n");
        exit(EXIT_FAILURE);
    }
    return sockfd;
}

void receive_request(int listener_d) {
    struct sockaddr_in client_address;
    socklen_t length = sizeof(client_address);
    while(1){
        int* client_descriptor = (int*)malloc(sizeof(int));
        *client_descriptor = accept(listener_d, (struct sockaddr*)&client_address, &length);

        pthread_t pid;
        pthread_create(&pid, NULL, (void*)process_request, (void*) (client_descriptor));
    }
}

void* process_request(void* client_descriptor) {
    chdir("/");
    int sockfd = *(int*)client_descriptor;

    char command[COMMAND_LENGTH] = {'\0'};
    char args[ARGS_LENGTH] = {'\0'};
    char buffer[BUFFER_LENGTH] = {'\0'};

    Session* state = (Session*)malloc(sizeof(Session));
    memset(state, 0, sizeof(state));
    state->sockfd = sockfd;

    write(sockfd, welcome_msg, strlen(welcome_msg));
    
    int read_bytes = read(sockfd, buffer, BUFFER_LENGTH);
    while (read_bytes > 0 && read_bytes <= BUFFER_LENGTH) {
        printf("Command: %s\n", buffer);
        sscanf(buffer,"%s %s", command, args);
        process_command(command, args, state);
        memset(buffer, '\0', sizeof(char) * BUFFER_LENGTH);
        memset(command, '\0', sizeof(char) * COMMAND_LENGTH);
        memset(args, '\0', sizeof(char) * ARGS_LENGTH);
        read_bytes = read(sockfd, buffer, BUFFER_LENGTH);
    }

    close(sockfd);
    free(state);
    printf("Client disconnected!\n");

    return NULL;
}

void process_command(char* command, char* args, Session* state) {
    int cmdlist_count = sizeof(cmdlist_str) / sizeof(char *);
    int command_index = -1;
    for (int i = 0; i < cmdlist_count; i++) {
        if (strcmp(command, cmdlist_str[i]) == 0) {
            command_index = i;
            break;
        }
    }
    switch (command_index) {
        case USER: command_user(args, state); break;
        case PASS: command_pass(args, state); break;
        case RETR: command_retr(args, state); break;
        case STOR: command_stor(args, state); break;
        case QUIT: command_quit(args, state); break;
        case SYST: command_syst(args, state); break;
        case TYPE: command_type(args, state); break;
        case PORT: command_port(args, state); break;
        case PASV: command_pasv(args, state); break;
        case MKD: command_mkd(args, state); break;
        case CWD: command_cwd(args, state); break;
        case PWD: command_pwd(args, state); break;
        case LIST: command_list(args, state); break;
        case RMD: command_rmd(args, state); break;
        case RNFR: command_rnfr(args, state); break;
        case RNTO: command_rnto(args, state); break;
        case ABOR: command_abor(args, state); break;
        case DELE: command_dele(args, state); break;
        case CDUP: command_cdup(args, state); break;
        default: {
            write(state->sockfd, unknown_command_msg, sizeof(unknown_command_msg));
            break;
        }
    }
}
