#include "strutils.h"
#include "netutils.h"

int main(int argc, char *argv[]) {
    check_root_permission();

    read_config();
    get_paras(argc, argv);

    int listen_fd = create_socket(config.listen_port, NULL);
    if (listen_fd < 0) {
        exit(EXIT_FAILURE);
    }

    log_record_start();

    receive_request(listen_fd);
    
    close(listen_fd);

    free_config();
    return 0;
}