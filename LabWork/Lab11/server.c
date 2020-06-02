#include <arpa/inet.h>
#include <dirent.h>
#include <inttypes.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "common.h"

int read_len(int fd, void *buf, size_t length);

/**
 * Function: read_len
 * Purpose:  reads from an input stream, retrying until a specific number of
 *           bytes has been read. This ensures complete message delivery.
 *
 * Args:
 *  * fd     - the file descriptor to read from
 *  * buf    - pointer to buffer to store data
 *  * length - size of the incoming message. If less than 'length' bytes are
 *             received, we'll keep retrying the read() operation.
 */
int read_len(int fd, void *buf, size_t length) {
    int result = 0;
    while (result < length) {
        result += read(fd, buf, length);
    }
    return result;
};

int main(int argc, char *argv[]) {

    if (argc != 2) {
        printf("Usage: %s port\n", argv[0]);
        return 1;
    }

    int port = atoi(argv[1]);

    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd == -1) {
        perror("socket");
        return 1;
    }

    struct sockaddr_in addr = { 0 };
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);
    if (bind(socket_fd, (struct sockaddr *) &addr, sizeof(addr)) == -1) {
        perror("bind");
        return 1;
    }

    if (listen(socket_fd, 10) == -1) {
        perror("listen");
        return 1;
    }

    LOG("Listening on port %d\n", port);

    while (true) {
        /* Outer loop: this keeps accepting connection */
        struct sockaddr_in client_addr = { 0 };
        socklen_t slen = sizeof(client_addr);

        int client_fd = accept(
                socket_fd,
                (struct sockaddr *) &client_addr,
                &slen);

        if (client_fd == -1) {
            perror("accept");
            return 1;
        }

        char remote_host[INET_ADDRSTRLEN];
        inet_ntop(
                client_addr.sin_family,
                (void *) &((&client_addr)->sin_addr),
                remote_host,
                sizeof(remote_host));
        LOG("Accepted connection from %s:%d\n", remote_host, client_addr.sin_port);


        while (true) {
            /* Inner loop: this keeps pulling data from the client */
            char size[128] = { 0};
            //get size of incoming msg from client.
            ssize_t sent_size = read_len(client_fd, size, sizeof(char*));
            int sz = atoi(size);//convert back to int

            //then do normal msg handling
            char buf[128] = {0};
            int bytes = 0;
            bytes = read_len(client_fd, buf, sz);//but with the sent size instead (and thats it.)

            if (bytes == -1) {
                perror("read");
                break;
            } else if (bytes == 0) {
                /* EOF */
                LOG("%s\n", "Reached end of stream");
                break;
            }

            printf("-> %s\n", buf);
        }
    }
}