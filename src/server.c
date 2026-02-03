#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>
#include "worker.h"
#include <netinet/in.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <sys/un.h>  // unix sockets
#include <unistd.h>

#define WORKERS 3
#define CONTROL_BUFSIZE 128
#define IPC_SOCKET_PATH "/tmp/server.control"

int make_unix_socket() {
    int control_fd;
    struct sockaddr_un addr;

    // 1. Create the socket (AF_UNIX instead of AF_INET)
    control_fd = socket(AF_UNIX, SOCK_STREAM, 0);

    // 2. Define the file path for the socket
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, IPC_SOCKET_PATH, sizeof(addr.sun_path)-1);

    // 3. Clean up any old socket file, then bind
    unlink(IPC_SOCKET_PATH);
    if (bind(control_fd, (struct sockaddr*)&addr, sizeof(addr)) != 0) {
        fprintf(stderr, "Could not bind to socket %s\n", IPC_SOCKET_PATH);
        return -1;
    }
    listen(control_fd, 5);

    return control_fd;
}

static sig_atomic_t run_server = 1;

void control_sig_handler(int sig) {
    (void)sig;
    run_server = 0;
}

void control_set_sig_handler() {
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = control_sig_handler;
    if(
        sigaction(SIGTERM, &sa, NULL) < 0 ||
        sigaction(SIGINT, &sa, NULL) < 0
    ) {
        perror("sigaction failed");
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char *argv[])
{
    const int port = (argc > 1) ? atoi(argv[1]) : 8080;

    int workers[WORKERS] = {0};

    int control_fd = make_unix_socket();
    int terminate_fd = 0;

    // set up objects waiting for kernel events
    struct pollfd pfd[2];
    pfd[0].fd = control_fd;
    pfd[0].events = POLLIN;
    pfd[1].fd = terminate_fd;
    pfd[1].events = POLLIN;

    control_set_sig_handler();

    int reload_needed = true;
    char buffer[CONTROL_BUFSIZE];
    while (1) {
        if (reload_needed || !run_server) {
            for (int i = 0; i < WORKERS; i++) {
                if (workers[i] > 0)
                    kill(workers[i], SIGTERM);

                if (run_server)
                    workers[i] = fork();

                if (workers[i] == 0) {
                    start_worker(port);
                    exit(EXIT_SUCCESS);
                }
            }
            reload_needed = false;
        }

        // wait for reload events or kill signal
        if (run_server)
            poll(pfd, 2, -1);
        else
            break;

        // handle reload request from unix socket
        if (pfd[0].revents & POLLIN) {
            int client_fd = accept(control_fd, NULL, NULL);
            int n = read(client_fd, buffer, sizeof(buffer)-1);
            buffer[n] = '\0';

            if (strncmp(buffer, "RELOAD", 6) == 0) {
                reload_needed = true;
                write(client_fd, "OK\n", 3);
            } else {
                write(client_fd, "UNKNOWN COMMAND\n", 16);
            }
            close(client_fd);

            fprintf(stderr, "Received reload request. Restarting workers.\n");
        }
    }

    fprintf(stderr, "Waiting for all workers to exit.\n");
    for (int i = 0; i < WORKERS; i++) {
        if (workers[i] > 0)
            waitpid(workers[i], NULL, 0);
    }

    return EXIT_SUCCESS;
}
