#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>

#define BUFFER_SIZE 1024

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_writer = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond_reader = PTHREAD_COND_INITIALIZER;

int writer_count = 0;
int reader_count = 0;
int database = 0;
int* viewer;

void* client_thread(void* arg) {
    int client_socket = *((int*)arg);
    int viewer_socket = *((int*)viewer);
    int wait = 2;
    char buffer[BUFFER_SIZE];
    while(1) {
// Lock the mutex
    pthread_mutex_lock(&mutex);
    // Receive data from the client
    memset(buffer, 0, BUFFER_SIZE);
    recv(client_socket, buffer, BUFFER_SIZE, 0);
    if (buffer[0] != 'r') {
        database = atoi(buffer);
        wait = 0;
        send(viewer_socket, "writer is writing", strlen("writer is writing"), 0);
    } else {
        send(viewer_socket, "reader is reading", strlen("reader is reading"), 0);
    }
    // Send the current database value to the client
    sprintf(buffer, "%d", database);
    send(client_socket, buffer, strlen(buffer), 0);

    // Unlock the mutex
    pthread_mutex_unlock(&mutex);
    sleep(wait);
    }
    
    // Close the client socket
    close(client_socket);

    pthread_exit(NULL);
}

void start_server(const char* host, int port) {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    // Create socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Set socket options
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("Setsockopt failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    // Bind the socket to the specified address and port
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    // Start listening for incoming connections
    if (listen(server_fd, 3) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on %s:%d\n", host, port);

    if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            perror("Accept failed");
            exit(EXIT_FAILURE);
        } else {
            printf("Accepted viewer\n");
        }
        viewer = malloc(sizeof(int));
        *viewer = new_socket;

    if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            perror("Accept failed");
            exit(EXIT_FAILURE);
        } else {
            printf("Accepted writer\n");
        }

        // Create a thread for the client
        pthread_t thread;
        int* client_socket = malloc(sizeof(int));
        *client_socket = new_socket;

        if (pthread_create(&thread, NULL, client_thread, client_socket) != 0) {
            perror("Thread creation failed");
            exit(EXIT_FAILURE);
        }

        // Detach the thread
        pthread_detach(thread);

    // Accept incoming connections and create threads for handling clients
    for (int i = 0; i < 3; ++i) {
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            perror("Accept failed");
            exit(EXIT_FAILURE);
        } else {
            printf("Accepted reader number %d\n", i + 1);
        }

        // Create a thread for the client
        pthread_t thread;
        int* client_socket = malloc(sizeof(int));
        *client_socket = new_socket;

        if (pthread_create(&thread, NULL, client_thread, client_socket) != 0) {
            perror("Thread creation failed");
            exit(EXIT_FAILURE);
        }

        // Detach the thread
        pthread_detach(thread);
    }
    while(1) {
    }
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s <host> <port>\n", argv[0]);
        return 1;
    }

    const char* host = argv[1];
    int port = atoi(argv[2]);

    start_server(host, port);

    return 0;
}

