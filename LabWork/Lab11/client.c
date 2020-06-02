#include <netdb.h> 
#include <netinet/in.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "common.h"

int main(int argc, char *argv[]) {

    if (argc != 3) {
       printf("Usage: %s hostname port\n", argv[0]);
       return 1;
    }

    char *server_hostname = argv[1];
    int port = atoi(argv[2]);

    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd == -1) {
        perror("socket");
        return 1;
    }

    struct hostent *server = gethostbyname(server_hostname);
    if (server == NULL) {
        fprintf(stderr, "Could not resolve host: %s\n", server_hostname);
        return 1;
    }

    struct sockaddr_in serv_addr = { 0 };
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    serv_addr.sin_addr = *((struct in_addr *) server->h_addr);

    if (connect(
                socket_fd,
                (struct sockaddr *) &serv_addr,
                sizeof(struct sockaddr_in)) == -1) {

        perror("connect");
        return 1;
    }

    LOG("Connected to server %s:%d\n", server_hostname, port);

    printf("Welcome. Please type your message below, or press ^D to quit.\n");

    while (true) {
        printf("message> ");
        fflush(stdout);

        char buf[128] = { 0 };
        char *str = fgets(buf, 128, stdin);
        if (str == NULL) {
            LOG("%s", "Reached EOF! Quitting.\n");
            break;
        }

        /* Remove newline characters */
        strtok(buf, "\r\n");

        size_t bytes_written = 0;

        char *write_ptr = str;

        /* Save size of string for later */
        int sz = strlen(str) + 1;

        size_t bytes_left = sz;

        /* Convert int to string */
        char *size_str;
        sprintf(size_str, "%d", sz);

        /* Send the size */
        int ret = write(socket_fd, size_str, sizeof(char*));

        if (ret == -1){
            perror("size write error");
        }

        do {
            bytes_written = write(socket_fd, write_ptr, sz);
            if (bytes_written == -1) {
                perror("write");
                return 1;
            }

            bytes_left = bytes_left - bytes_written;
            write_ptr += bytes_written;
            LOG("Wrote %zd bytes\n", bytes_written);
        } while (bytes_left > 0);
    }

    return 0;
}