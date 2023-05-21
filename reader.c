#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 1024

int name;

void start_client(const char* server_ip, int server_port) {
    int sock = 0, valread;
    struct sockaddr_in serv_addr;
    char buffer[BUFFER_SIZE] = {0};

    // Create socket file descriptor
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(server_port);

    // Convert IPv4 and IPv6 addresses from text to binary form
    if (inet_pton(AF_INET, server_ip, &serv_addr.sin_addr) <= 0) {
        perror("Invalid address/Address not supported");
        exit(EXIT_FAILURE);
    }

    // Connect to the server
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Connection failed");
        exit(EXIT_FAILURE);
    }

    printf("Connected to the server\n");
    while(1) {
        // Send a transaction request to the server
        send(sock, "read", strlen("read"), 0);
       // Receive the current database value from the server
       memset(buffer, 0, BUFFER_SIZE);
       valread = recv(sock, buffer, BUFFER_SIZE, 0);
       if (valread > 0) {
           printf("Reader number %d read: %s\n", name, buffer);
       } else {
           printf("Read failed\n");
       }
    }

    // Close the socket
    close(sock);
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        printf("Port num and name required");
        return 1;
    }

    const char* server_ip = argv[1];
    int server_port = atoi(argv[2]);
    name = atoi(argv[3]);

    start_client(server_ip, server_port);

    return 0;
}

