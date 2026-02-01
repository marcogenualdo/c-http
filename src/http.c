#include "responses.h"
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include "http.h"

HttpRequest parse_request(char *buffer) {
    HttpRequest request;
    sscanf(buffer, "%s %s", request.method, request.path);
    return request;
}

int read_request(int client_fd, char *buffer) {
    int n, read_bytes = 0;
    while (1) {
        // reading from socket
        n = read(client_fd, buffer, BUFSIZE - read_bytes - 1);
        if (n <= 0) {
            perror("Error reading from socket, client disconnected");
            return 1;
        }

        // checking buffer overflow
        read_bytes += n;
        if (read_bytes >= BUFSIZE - 1) {
            write(client_fd, HTTP_ERR_414, strlen(HTTP_ERR_414));
            return 1;
        }
        buffer[read_bytes + n] = '\0';

        // checking for end of request
        if (strstr(buffer, "\r\n\r\n")) break;
    }

    write(client_fd, HTTP_OK, strlen(HTTP_OK));
    close(client_fd);
    return 0;
}
