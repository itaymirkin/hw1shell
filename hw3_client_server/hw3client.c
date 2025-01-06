#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#define BUFFER_SIZE 1024
void *recv_func(void *arg) {
    int client_socket = *(int *)arg;
    char buffer[BUFFER_SIZE];
    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        int len = recv(client_socket, buffer, BUFFER_SIZE, 0);
        if (len < 0) {
            perror("recv");
            break;
        }
        if (len == 0) {
            printf("Disconnected from server.\n");
            exit(EXIT_SUCCESS);;
        }
        printf("%s\n", buffer);
    }
    return NULL;
}
int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <server IP> <server port> <client name>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    const char *server_addr= argv[1];
    int server_port = atoi(argv[2]);
    const char *client_name = argv[3];
    //printf("Connecting to server %s:%d as '%s'.\n", server_addr, server_port, client_name); - debug

    // Create socket
    int client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket < 0) {
        perror("client socket");
        exit(EXIT_FAILURE);
    }   
    //Set up the server address struct
    struct sockaddr_in server;
    server.sin_family = AF_INET;   
    //Convert port number to network byte order
    server.sin_port = htons(server_port);
    if (inet_pton(AF_INET, server_addr, &server.sin_addr) < 0) {
        perror("inet_pton error");
        close(client_socket);
        exit(EXIT_FAILURE);
    }
    
   

    // Connect to server
    if (connect(client_socket, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("Connection failed");
        close(client_socket);
        exit(EXIT_FAILURE);
    }

    //send client name to server
    if(send(client_socket, client_name, strlen(client_name), 0) < 0) {
        perror("client send name");
        close(client_socket);
        exit(EXIT_FAILURE);
    }

    //printf("Connected to server as '%s'.\n", client_name); - debug

    pthread_t recv_thread;
    if(pthread_create(&recv_thread, NULL, recv_func, (void *)&client_socket) != 0) {
        perror("pthread_create");
        close(client_socket);
        exit(EXIT_FAILURE);
    }
    //get user input and send to server
    char buffer[BUFFER_SIZE];
    while (1)
    {
        //reset buffer
        memset(buffer, 0, BUFFER_SIZE);
        if(fgets(buffer, BUFFER_SIZE, stdin) == NULL) {
            perror("fgets");
            break;
        }
        

        buffer[strcspn(buffer, "\n")] = '\0';
        //printf("Sending: %s\n", buffer); - debug
        //handle !exit command
        if (strcmp(buffer, "!exit") == 0) {
            printf("client exiting\n");
            break;
        }
        if (send(client_socket, buffer, strlen(buffer), 0) < 0) {
            perror("send");
            break;
        }
        
    }

    // Close socket
    close(client_socket);
    pthread_cancel(recv_thread);
    pthread_join(recv_thread, NULL);
    return 0;
}
    