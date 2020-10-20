#include "strutils.h"
#include "netutils.h"

int main(int argc, char *argv[]) {
    check_root_permission();

    read_config();
    get_paras(argc, argv);

    int listen_fd = create_ftp_server();

    int ip[4] = { 0 };
    get_local_ip(ip);

    printf("FTP server start!\r\n");
    printf("listen_port: %u\r\n", config.listen_port);
    printf("listen_address: %d.%d.%d.%d\r\n", ip[0], ip[1], ip[2], ip[3]);
    printf("root_path: %s\r\n", config.root_path);
    printf("listen_fd: %d\r\n", listen_fd);

    receive_request(listen_fd);
    close(listen_fd);

    free_config();
    return 0;
}