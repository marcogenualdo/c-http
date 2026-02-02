#include <stdio.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include "control.h"
#include "http.h"

#define MAXCONN 10

int make_tcp_socket(int port) {
    int tcp = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr = {
        .sin_family = AF_INET,
        .sin_addr.s_addr = INADDR_ANY,
        .sin_port = htons(port)
    };
    int optval = 1;
    // Allow rapid restarts (TIME_WAIT reuse)
    setsockopt(tcp, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

    // Allow multiple processes to bind to the same port (Kernel load balancing)
    if (setsockopt(tcp, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval)) < 0) {
        perror("setsockopt(SO_REUSEPORT) failed");
        exit(EXIT_FAILURE);
    }
    if (bind(tcp, (struct sockaddr*)&addr, sizeof(addr))) {
        fprintf(stderr, "Could not bind to port %d\n", port);
        exit(EXIT_FAILURE);
    }
    listen(tcp, MAXCONN);

    return tcp;
}

void listener(int tcp) {
    handle_req_t handle_request_func = load_handlers();

    char buffer[BUFSIZE];
    while(1) {
        int client = accept(tcp, NULL, NULL);

        read_request(client, buffer);
        HttpRequest request = parse_request(buffer);
        fprintf(stderr, "Request: %s %s\n", request.method, request.path);

        char *response = handle_request_func(request.path);
        write(client, response, strlen(response));
        free(response);
        close(client);
    }
}


void start_worker(int port) {
    listener(make_tcp_socket(port));
}
