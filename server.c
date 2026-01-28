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
#include "responses.h"

#define HTTP_HEADER_BUFFER_SIZE 4096

void* handle_client(void* arg) {
    int client_fd = *(int*)arg;
    free(arg);

    // READ LOCK: Multiple threads can hold this simultaneously
    pthread_rwlock_rdlock(&reload_lock);

    // Read request
    char buffer[HTTP_HEADER_BUFFER_SIZE];
    int total_read = 0;

    while (1) {
        // Reading stream
        int n = read(
            client_fd,  // where to read
            buffer + total_read,  // pointer to write start
            sizeof(buffer) - total_read - 1  // now many bytes can write
        );

        if (n <= 0) break; // Error or closed

        total_read += n;
        buffer[total_read] = '\0'; // We safely have room for this because of the -1 above

        // Checking if header block is fininshed
        if (strstr(buffer, "\r\n\r\n")) {
            break;
        }

        // OVERFLOW: Buffer is totally full, but still no \r\n\r\n found
        if (total_read >= sizeof(buffer) - 1) {
            write(client_fd, HTTP_ERR_431, strlen(HTTP_ERR_431));
            break;
        }
    }

    // Mini request parser
    char *method = buffer;
    char *path = strchr(buffer, ' ');
    if (path) {
        *path = '\0'; // Terminate "GET"
        path++;       // Move to start of path

        char *end_path = strchr(path, ' ');
        if (end_path) {
            *end_path = '\0'; // Terminate "/logic/test"
        }
    }

    if (path != NULL) {
        printf("[Thread %lu] Processing %s %s\n", pthread_self(), method, path);

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
