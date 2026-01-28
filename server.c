#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

// A simple struct to pass data to our thread
typedef struct {
    int client_fd;
} thread_args_t;



// This function is what the new "Task" will execute
void* handle_client(void* args) {
    thread_args_t* t_args = (thread_args_t*)args;
    int fd = t_args->client_fd;
    free(t_args); // Free the struct memory allocated in main

    char buffer[1024] = {0};
    read(fd, buffer, 1024);

    // Demonstrate shared memory: Every thread can see this same string
    char *response = "HTTP/1.1 200 OK\nContent-Length: 18\n\nHello from Thread!";
    write(fd, response, strlen(response));

    printf("[Thread %lu] Handled request on FD %d\n", pthread_self(), fd);

    close(fd);
    return NULL;
}

int main(int argc, char** argv) {
    int port = 8080;
    if (argc != 1) {
        port = atoi(argv[1]);
    }

    int server_fd;
    struct sockaddr_in addr = { .sin_family = AF_INET, .sin_port = htons(port), .sin_addr.s_addr = INADDR_ANY };

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) != 0) {
        perror("Failed to bind socket");
        return 1;
    }
    listen(server_fd, 10);
    printf("Listening on port %d\n", port);

    while(1) {
        int client_fd = accept(server_fd, NULL, NULL);

        // Prepare arguments for the thread
        thread_args_t *args = malloc(sizeof(thread_args_t));
        args->client_fd = client_fd;

        pthread_t thread_id;
        // pthread_create tells the kernel to spawn a new task sharing the address space
        if (pthread_create(&thread_id, NULL, handle_client, args) != 0) {
            perror("Failed to create thread");
            close(client_fd);
            free(args);
        }

        // We "detach" the thread so the OS cleans up its resources automatically when it ends
        pthread_detach(thread_id);
    }
    return 0;
}
