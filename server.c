#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <dlfcn.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

// Global function pointer (The shared "tool" for all threads)
char* (*handle_request_func)(char*);

void* handle_client(void* arg) {
    int client_fd = *(int*)arg;
    free(arg);

    char buffer[2048] = {0};
    read(client_fd, buffer, 2048);

    // --- MINI PARSER ---
    // Extract the path from "GET /index.html HTTP/1.1"
    // We look for the first space, then the second space.
    char *method = strtok(buffer, " ");
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

    close(client_fd);
    return NULL;
}

int main(int argc, char* argv[]) {
    int port = 8080;
    if (argc > 1) {
        port = atoi(argv[1]);
    }

    // 1. Load the Library at Startup
    void* handle = dlopen("./handlers.so", RTLD_LAZY);
    if (!handle) {
        fprintf(stderr, "Cannot load handlers.so: %s\n", dlerror());
        return 1;
    }
    handle_request_func = dlsym(handle, "handle_request");

    // 2. Setup Socket
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
        int client_fd = accept(server_fd, NULL, NULL);
        int *arg = malloc(sizeof(int));
        *arg = client_fd;

        pthread_t tid;
        pthread_create(&tid, NULL, handle_client, arg);
        pthread_detach(tid);
    }

    dlclose(handle);
    return 0;
}
