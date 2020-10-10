#include "strutils.h"
#include "netutils.h"

int main(int argc, char *argv[]) {
    check_root_permission();

    Config config = { 21, NULL, NULL };
    read_config(&config);
    get_paras(argc, argv, &config);

    int sockfd = create_ftp_server(&config);

    int ip[4] = { 0 };
    get_local_ip(sockfd, ip);

    printf("FTP server start!\n");
    printf("listen_port: %u\n", config.listen_port);
    printf("listen_address: %d.%d.%d.%d\n", ip[0], ip[1], ip[2], ip[3]);
    printf("root_path: %s\n", config.root_path);
    printf("sockfd: %d\n", sockfd);

    receive_request(sockfd);
    close(sockfd);

    free(config.listen_address);
    free(config.root_path);
    return 0;
}