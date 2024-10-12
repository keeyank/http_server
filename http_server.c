#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 1024
#define BACKLOG 3

void send_file(int socket, const char *filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        const char *not_found = "File not found\n";
        write(socket, not_found, strlen(not_found));
        return;
    }

    char buffer[BUFFER_SIZE];
    size_t bytes_read;
    while ((bytes_read = fread(buffer, 1, sizeof(buffer), file)) > 0) {
        write(socket, buffer, bytes_read);
    }

    fclose(file);
}

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char buffer[BUFFER_SIZE] = {0};
    char *hello = "HTTP/1.1 200 OK\nContent-Type: text/html\n\n<!DOCTYPE html><html><body><h1>Hello, World!</h1></body></html>";

    // Create socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Bind the socket to the network address and port
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // Start listening for connections
    if (listen(server_fd, BACKLOG) < 0) {
        perror("listen failed");
        exit(EXIT_FAILURE);
    }

    while(1) {
        printf("\nWaiting for a connection...\n");

        // Accept a client connection
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            perror("accept failed");
            exit(EXIT_FAILURE);
        }

        // Read the request
        read(new_socket, buffer, BUFFER_SIZE);
        printf("%s\n", buffer);

        // Send a response
        write(new_socket, hello, strlen(hello));
        printf("Hello message sent\n");

        close(new_socket);
    }

    return 0;
}
