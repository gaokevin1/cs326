#include <arpa/inet.h>
#include <dirent.h>
#include <fcntl.h>
#include <inttypes.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/sendfile.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h> 
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#include "debug.h"

/**
 * Retrieves the next token from a string.
 *
 * Parameters:
 * - str_ptr: maintains context in the string, i.e., where the next token in the
 *   string will be. If the function returns token N, then str_ptr will be
 *   updated to point to token N+1. To initialize, declare a char * that points
 *   to the string being tokenized. The pointer will be updated after each
 *   successive call to next_token.
 *
 * - delim: the set of characters to use as delimiters
 *
 * Returns: char pointer to the next token in the string.
 */
char *next_token(char **str_ptr, const char *delim)
{
    if (*str_ptr == NULL) {
        return NULL;
    }

    size_t tok_start = strspn(*str_ptr, delim);
    size_t tok_end = strcspn(*str_ptr + tok_start, delim);

    /* Zero length token. We must be finished. */
    if (tok_end  <= 0) {
        *str_ptr = NULL;
        return NULL;
    }

    /* Take note of the start of the current token. We'll return it later. */
    char *current_ptr = *str_ptr + tok_start;

    /* Shift pointer forward (to the end of the current token) */
    *str_ptr += tok_start + tok_end;

    if (**str_ptr == '\0') {
        /* If the end of the current token is also the end of the string, we
         * must be at the last token. */
        *str_ptr = NULL;
    } else {
        /* Replace the matching delimiter with a NUL character to terminate the
         * token string. */
        **str_ptr = '\0';

        /* Shift forward one character over the newly-placed NUL so that
         * next_pointer now points at the first character of the next token. */
        (*str_ptr)++;
    }

    return current_ptr;
}

/**
 * Generates an HTTP 1.1 compliant timestamp for use in HTTP responses.
 *
 * Inputs:
 *  - timestamp: character pointer to a string buffer to be filled with the
 *    timestamp.
 */
void generate_timestamp(char *timestamp, size_t length)
{
    time_t now = time(0);
    struct tm time = *gmtime(&now);
    strftime(timestamp, length, "%a, %d %b %Y %H:%M:%S %Z", &time);
}

/**
 * Reads from a file descriptor until:
 *  - the newline ('\n') character is encountered
 *  - *length* is exceeded
 *  This is helpful for reading HTTP headers line by line.
 *
 * Inputs:
 *  - fd: file descriptor to read from
 *  - buf: buffer to store data read from *fd*
 *  - length: maximum capacity of the buffer
 *
 * Returns:
 *  - Number of bytes read;
 *  - -1 on read failure
 *  - 0 on EOF
 */
ssize_t read_line(int fd, char *buf, size_t length) {
    ssize_t total = 0;

    while (total < length) {
        size_t read_sz = read(fd, buf + total, 1);
        if (*(buf + total) == '\n') {
            break;
        }
        if (read_sz == -1) {
            perror("read");
            return -1;
        } else if (read_sz == 0) {
            /* EOF */
            return 0;
        }
        total += read_sz;
    }
    return total;
}

int not_found(int client_fd) {
    char buf[8192] = { 0 };
    char ts[128];
    char *error = "404 Not Found";
    generate_timestamp(ts, 128);
    sprintf(buf, "HTTP/1.1 404 Not Found\r\n"
            "Date: %s\r\n"
            "Content-Length: %zu\r\n"
            "\r\n"
            "%s",
            ts, strlen(error), error);

    int ret = write(client_fd, buf, strlen(buf));
    if (ret == -1) {
        return -1;
    }
    return 0;
}

/**
 * TODO: reads an HTTP 1.1 request and responds with the appropriate file (or
 * 404 if the file does not exist).
 */
int handle_request(int client_fd) {
    LOGP("Handling Request\n");
   
    char path[8192] = {0};
    while(true) {
        char header[8192] = {0};
 
        ssize_t read_sz = read_line(client_fd, header, 8192);
        if (read_sz == -1 || read_sz == 0) {
            return read_sz;
        }
 
        LOG("-> %s", header);
        char *next_tok = header;
        char *curr_tok;
 
        curr_tok = next_token(&next_tok, " \t\r\n");
 
        if (curr_tok != NULL && strcmp(curr_tok, "GET") == 0) {
              curr_tok = next_token(&next_tok, " \t\r\n");
              path[0] = '.';
              strcpy(&path[1], curr_tok);
              LOG("URI: %s\n", curr_tok);
        } else if (curr_tok == NULL){
            break;
        }
    }
 
    LOG("File path: %s\n", path);
 
    struct stat sb;
    int ret = stat(path, &sb);
    if (ret == -1){
        perror("stat");
        not_found(client_fd);
        return 0;
    }
 
    if (S_ISDIR(sb.st_mode)) {
        strcat(path, "/index.html");
        ret = stat(path, &sb);
        if (ret == -1) {
            perror("stat");
            not_found(client_fd);
            return 0;
        }
    }
 
    char msg[8192] = { 0 };
    char date[128] = { 0 };
    generate_timestamp(date, 128);
    sprintf(msg,
                "HTTP/1.1 200 OK\r\n"
                "Date: %s\r\n"
                "Content-Length: %zu\r\n"
                "\r\n",
                date, sb.st_size);
   
    LOG("Sending response:\n%s", msg);
    write(client_fd, msg, strlen(msg));
   
    int file_fd = open(path, O_RDONLY);
    sendfile(client_fd, file_fd, 0, sb.st_size);
 
    return 0;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s port dir\n", argv[0]);
        return 1;
    }
 
    int port = atoi(argv[1]);
    char *dir = argv[2];
 
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
 
    LOG("changing directory to %s\n", dir);
    int ret = chdir(dir);
    if (ret == -1) {
        perror("chdir");
        return 1;
    }
 
    while (true) {
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
 
        pid_t pid = fork();
 
        if (pid == 0) {
            /* child */
            close(socket_fd);
            char remote_host[INET_ADDRSTRLEN];
            inet_ntop(
                    client_addr.sin_family,
                    (void *) &((&client_addr)->sin_addr),
                    remote_host,
                    sizeof(remote_host));
            LOG("Accepted connection from %s:%d\n", remote_host, client_addr.sin_port);
 
            while (true) {
                int ret = handle_request(client_fd);
                if (ret == -1 || ret == 0) {
                    break;
                }
            }
        } else if (pid < 0) {
            perror("fork");
            exit(1);
        } else {
            close(client_fd);
        }
    }
 
    return 0;
}
