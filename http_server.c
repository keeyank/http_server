#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 1024
#define BACKLOG 3
#define MAX_PATH_LENGTH 256

void send_file_with_headers(int socket, const char *filename) {
    FILE *file = fopen(filename, "r");

    if (file == NULL) {
        const char *not_found = "HTTP/1.0 404 Not Found\r\nContent-Type: text/plain\r\n\r\nFile not found\n";
        write(socket, not_found, strlen(not_found));
        printf("File not found: %s\n", filename);
        return;
    }

    // Determine content type based on file extension
    const char *content_type = "text/plain";
    char *dot = strrchr(filename, '.');
    if (dot) {
        if (strcmp(dot, ".html") == 0 || strcmp(dot, ".htm") == 0) {
            content_type = "text/html";
        } else if (strcmp(dot, ".css") == 0) {
            content_type = "text/css";
        } else if (strcmp(dot, ".js") == 0) {
            content_type = "application/javascript";
        }
    }

    // Send HTTP headers
    char headers[BUFFER_SIZE];
    snprintf(headers, sizeof(headers),
             "HTTP/1.0 200 OK\r\n"
             "Content-Type: %s\r\n"
             "\r\n", content_type);
    write(socket, headers, strlen(headers));

    // Send file content
    char buffer[BUFFER_SIZE];
    size_t bytes_read;
    while ((bytes_read = fread(buffer, 1, sizeof(buffer), file)) > 0) {
        write(socket, buffer, bytes_read);
    }

    printf("File sent successfully with headers: %s\n", filename);

    fclose(file);
}

int main() {
    int server_fd, client_socket;
    struct sockaddr_in address;
    socklen_t addrlen = sizeof(address);
    char buffer[BUFFER_SIZE] = {0};

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
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    // Start listening for connections
    if (listen(server_fd, BACKLOG) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    while(1) {
        printf("\nWaiting for a connection...\n");

        // Accept a client connection
        if ((client_socket = accept(server_fd, (struct sockaddr *)&address, &addrlen)) < 0) {
            perror("Accept failed");
            exit(EXIT_FAILURE);
        }

        // Read the request
        ssize_t bytes_read = read(client_socket, buffer, BUFFER_SIZE - 1);
        if (bytes_read == 0) {continue;}

        buffer[bytes_read] = '\0';
        printf("Received request:\n%s\n\n", buffer);

        char method[16], path[MAX_PATH_LENGTH];
        sscanf(buffer, "%15s %255s", method, path);

        // HTTP 0.9 parsing for now, only implements GET method
        if (strcmp(method, "GET") == 0) {

            // Prepend 'site/' to the file path
            char full_path[MAX_PATH_LENGTH];
            snprintf(full_path, sizeof(full_path), "site%s", path);

            // Handle root path
            if (strcmp(path, "/") == 0) {
                snprintf(full_path, sizeof(full_path), "site/index.html");
            }

            send_file_with_headers(client_socket, full_path);
        } else {
            const char *error = "HTTP/1.0 501 Not Implemented\r\nContent-Type: text/plain\r\n\r\nOnly GET method is supported\n";
            printf("Sent response:\n%s\n", error);
            write(client_socket, error, strlen(error));
        }

        close(client_socket);
    }

    return 0;
}
