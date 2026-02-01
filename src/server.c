#include <stdio.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <sys/socket.h>
#include "http.h"

#define MAXCONN 10

// control thread
// open unix socket
// on RELOAD signal, restart all workers, or tell them to reload the plugin

// plugin
// search file cache
// if not found, load static file
// build response
// return response to worker

// worker
// load plugin with dlopen
// get from unix socket
// process HTTP request: method, path, headers
// dispatch to plugin

int main(int argc, char *argv[])
{
    const int port = (argc > 1) ? atoi(argv[1]) : 8080;

    // spawn control thread
    // spawn worker processes
    int tcp = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr = {
        .sin_family = AF_INET,
        .sin_addr.s_addr = INADDR_ANY,
        .sin_port = htons(port)
    };
    int optval = 1;
    if (setsockopt(tcp, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0) {
        perror("setsockopt(SO_REUSEADDR) failed");
        exit(EXIT_FAILURE);
    }
    if (bind(tcp, (struct sockaddr*)&addr, sizeof(addr)))
        fprintf(stderr, "Could not bind to port %d", port);
    listen(tcp, MAXCONN);

    char buffer[BUFSIZE];
    while(1) {
        int client = accept(tcp, NULL, NULL);
        read_request(client, buffer);
        HttpRequest request = parse_request(buffer);
        fprintf(stderr, "Request: %s %s\n", request.method, request.path);
    }
    //
    // start data loop
    // read from socket
    // dispatch to worker
    return EXIT_SUCCESS;
}
