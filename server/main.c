#include "strutils.h"
#include "netutils.h"

int main(int argc, char *argv[]) {
    check_root_permission();
    getcwd(code_path, PATH_LENGTH);

    read_config();
    get_paras(argc, argv);

    int listen_fd = create_socket(config.listen_port, NULL);
    if (listen_fd < 0) {
        exit(EXIT_FAILURE);
    }

    int ip[4] = { 0 };
    get_local_ip(ip);

    printf("FTP server start!\r\n");
    printf("Listen Port: %u\r\n", config.listen_port);
    printf("Local IP Address: %d.%d.%d.%d\r\n", ip[0], ip[1], ip[2], ip[3]);
    printf("Root Path: %s\r\n", config.root_path);

    receive_request(listen_fd);
    
    close(listen_fd);

    free_config();
    return 0;
}