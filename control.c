#include <dlfcn.h> // dlopen, dlsym
#include <pthread.h> // pthread_create
#include <stdatomic.h> // thread safe assignemnts
#include <stdbool.h> // can assign true and false
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h> // network sockets
#include <sys/un.h>  // unix sockets
#include <unistd.h>
#include "handlers.h"

#define IPC_SOCKET_PATH "/tmp/server.control"

// Global Plugin State
void* lib_handle = NULL;
handle_req_t handle_request_func = NULL;

// Locking plugin reload
volatile atomic_bool reload_needed = false;
pthread_rwlock_t reload_lock = PTHREAD_RWLOCK_INITIALIZER;

void load_plugin() {
    // Write lock ensures no one is using the old pointer while swap happens
    pthread_rwlock_wrlock(&reload_lock);

    if (lib_handle) dlclose(lib_handle);

    lib_handle = dlopen("./handlers.so", RTLD_LAZY);
    if (!lib_handle) {
        fprintf(stderr, "Failed to load: %s\n", dlerror());
        exit(1);
    }
    handle_request_func = dlsym(lib_handle, "handle_request");

    pthread_rwlock_unlock(&reload_lock);
    printf("Plugin loaded/reloaded successfully.\n");
}

void* control_thread_func(void* arg) {
    (void)arg; // unused but necessary for pthread syscall

    int control_fd;
    struct sockaddr_un addr;
    char buffer[100];

    // 1. Create the socket (AF_UNIX instead of AF_INET)
    control_fd = socket(AF_UNIX, SOCK_STREAM, 0);

    // 2. Define the file path for the socket
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, IPC_SOCKET_PATH, sizeof(addr.sun_path)-1);

    // 3. Clean up any old socket file, then bind
    unlink(IPC_SOCKET_PATH);
    if (bind(control_fd, (struct sockaddr*)&addr, sizeof(addr)) != 0) {
        printf("Could not bind to socket %s\n", IPC_SOCKET_PATH);
        return NULL;
    }
    listen(control_fd, 5);

    printf("Control socket ready at %s\n", IPC_SOCKET_PATH);

    while (1) {
        int client_fd = accept(control_fd, NULL, NULL);
        int n = read(client_fd, buffer, sizeof(buffer)-1);
        buffer[n] = '\0';

        if (strncmp(buffer, "RELOAD", 6) == 0) {
            reload_needed = true; // Set our atomic flag
            write(client_fd, "OK\n", 3);
        } else {
            write(client_fd, "UNKNOWN COMMAND\n", 16);
        }
        close(client_fd);
    }
    return NULL;
}
