#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>
#include "worker.h"

#define WORKERS 3

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
    for (int i = 0; i < WORKERS; i++) {
        if (fork() == 0) {
            start_worker(port);
        }
    }
    fprintf(stderr, "Started %d workers.\n", WORKERS);

    while (1) {
        int status;
        pid_t done = wait(&status);
        if (done == -1) {
            break;
        }
    }

    //
    // start data loop
    // read from socket
    // dispatch to worker
    return EXIT_SUCCESS;
}
