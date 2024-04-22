#define _XOPEN_SOURCE 500 // Required for nftw
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#define PORT 7000
#define SERVER_IP "127.0.0.1" // Change this to the server's IP address
#define MAX_COMMAND_SIZE 256

int main(int argc, char *argv[]) {
    int client_socket;
    struct sockaddr_in server_addr;
    char command[MAX_COMMAND_SIZE];

    // Create socket
    if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Server configurations
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);
    server_addr.sin_port = htons(PORT);

    // Connect to server
    if (connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("Connection failed");
        exit(EXIT_FAILURE);
    }
    send(client_socket, "true", 4, 0);
    char buffer[4] = {0};
    recv(client_socket, buffer, 4, 0);
    int port = atoi(buffer);
    printf("Client got this port %d\n", port);
    int client_socket2;
    struct sockaddr_in server_addr2;

    // struct sockaddr_in server_addr2;

    // Create socket
    if ((client_socket2 = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Server configurations
    server_addr2.sin_family = AF_INET;
    server_addr2.sin_addr.s_addr = inet_addr(SERVER_IP);
    server_addr2.sin_port = htons(port);

    if (connect(client_socket2, (struct sockaddr *)&server_addr2, sizeof(server_addr2)) == -1) {
        perror("Connection failed");
        exit(EXIT_FAILURE);
    }

    if(port == 7000){
        send(client_socket2, "false", 4, 0);
    }

    printf("Connected to server.\n");

    while (1) {
        printf("clientw24$: ");
        fgets(command, sizeof(command), stdin);
        command[strcspn(command, "\n")] = 0; // Remove trailing newline
        
        // Send command to server
        send(client_socket2, command, strlen(command), 0);
        
        if (strncmp(command, "quitc", 5) == 0) {
            printf("Exiting from appropriate server\n");
            break;
        }
        
        // Receive response from server
        char buffer[1024] = {0};
        recv(client_socket2, buffer, sizeof(buffer), 0);
        printf("Server response:\n%s\n", buffer);
    }

    // Close socket
    close(client_socket2);

    return 0;
}