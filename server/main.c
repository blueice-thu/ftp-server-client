#include "strutils.h"
#include "netutils.h"

int main(int argc, char *argv[]) {
    check_root_permission();
    read_config();
    get_paras(argc, argv);
    
    printf("FTP server start!\n");
    printf("listen_port: %u\n", listen_port);
    printf("listen_address: %s\n", listen_address);
    printf("root_path: %s\n", root_path);

    int sockfd = create_ftp_server(listen_address, listen_port);
    printf("sockfd: %d\n", sockfd);
    receive_request(sockfd);

    free(listen_address);
    free(root_path);
    return 0;
}