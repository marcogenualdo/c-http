#ifndef HTTP_H
#define HTTP_H

#define BUFSIZE 1024

typedef struct http_request {
    char *method;
    char *path;
} HttpRequest;

HttpRequest parse_request(char *buffer);
int read_request(int client_fd, char *buffer);

#endif
