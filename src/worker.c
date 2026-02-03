#include <dlfcn.h>
#include <signal.h>
#include <stdatomic.h>
#include <stdio.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <unistd.h>
#include "requests.h"

#define MAXCONN 10

void* lib_handle = NULL;

typedef char* (*handle_req_t)(char*);

handle_req_t load_handlers() {
    handle_req_t handle_request_func = NULL;

    if (lib_handle) dlclose(lib_handle);

    lib_handle = dlopen("libchttp-handlers.so", RTLD_LAZY);
    if (!lib_handle) {
        fprintf(stderr, "Failed to load: %s\n", dlerror());
        exit(1);
    }
    handle_request_func = dlsym(lib_handle, "handle_request");

    return handle_request_func;
}

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

static sig_atomic_t terminate = 0;

void sig_handler(int sig) {
    (void)sig;
    terminate = 1;
}

void set_sig_handler() {
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = sig_handler;
    if(sigaction(SIGTERM, &sa, NULL) < 0) {
        perror("sigaction failed");
        exit(EXIT_FAILURE);
    }
}

void listener(int tcp) {
    handle_req_t handle_request_func = load_handlers();

    // set up objects waiting for kernel events
    struct pollfd pfd[2];
    pfd[0].fd = tcp;
    pfd[0].events = POLLIN;
    pfd[1].fd = terminate;
    pfd[1].events = POLLIN;

    set_sig_handler();

    char buffer[BUFSIZE];
    while(1) {
        poll(pfd, 2, -1);

        // received SIGTERM
        if (terminate) break;

        // received TCP data
        if (pfd[0].revents & POLLIN) {
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
}


void start_worker(int port) {
    listener(make_tcp_socket(port));
}
