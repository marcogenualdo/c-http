#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include "control.h"

void* handle_client(void* arg) {
    int client_fd = *(int*)arg;
    free(arg);

    // READ LOCK: Multiple threads can hold this simultaneously
    pthread_rwlock_rdlock(&reload_lock);

    char buffer[2048] = {0};
    read(client_fd, buffer, 2048);

    // --- MINI PARSER ---
    // Extract the path from "GET /index.html HTTP/1.1"
    // We look for the first space, then the second space.
    strtok(buffer, " ");
    char *path = strtok(NULL, " ");

    if (path != NULL) {
        printf("[Thread %lu] Processing path: %s\n", pthread_self(), path);

        // CALL THE PLUGIN
        // This is the "ABI Call". We are jumping to code
        // that wasn't here when we compiled the server.
        char *response = handle_request_func(path);

        write(client_fd, response, strlen(response));
        free(response); // The plugin used malloc, so we must free!
    }

    pthread_rwlock_unlock(&reload_lock);

    close(client_fd);
    return NULL;
}

int main(int argc, char* argv[]) {
    // Select port
    int port = 8080;
    if (argc > 1) {
        port = atoi(argv[1]);
    }

    // Start dynamic lib loading thread
    pthread_t control_tid;
    pthread_create(&control_tid, NULL, control_thread_func, NULL);
    reload_needed = true;

    // Setup Socket
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr = {
        .sin_family = AF_INET,
        .sin_port = htons(port),
        .sin_addr.s_addr = INADDR_ANY
    };
    if (bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) != 0) {
        printf("Could not bind to port %d", port);
        return 1;
    }
    listen(server_fd, 10);

    while(1) {
        // Before accepting a new connection, check if handlers need to be updated
        if (reload_needed) {
            reload_needed = false;
            load_plugin(); // Now we reload safely in the main thread
        }

        int client_fd = accept(server_fd, NULL, NULL);
        int *arg = malloc(sizeof(int));
        *arg = client_fd;

        pthread_t tid;
        pthread_create(&tid, NULL, handle_client, arg);
        pthread_detach(tid);
    }
    return 0;
}
