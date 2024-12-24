#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

#define MAX_CLIENTS 100
#define BUFFER_SIZE 1024
#define NAME_SIZE 32
#define INET_ADDR_STRLEN 16

typedef struct {
    int socket;
    char name[NAME_SIZE];
    char ip[INET_ADDR_STRLEN];
} client_t;

client_t clinets [MAX_CLIENTS];
int nof_clients = 0;
pthread_mutex_t clinets_mutex = PTHREAD_MUTEX_INITIALIZER;

handle_clinet_message(client_t *client, char *message, int len) {
   if (message[0] == '@')
   {
    //Parse whisper message
    char *whisper = strtok(message + 1, " ");
    char whisper_reciver[NAME_SIZE];
    char whisper_message[BUFFER_SIZE];
    strncpy(whisper_reciver, whisper[0], NAME_SIZE);
    strncpy(whisper_message, whisper[1], BUFFER_SIZE);

    //find socket of the reciver
    int reciver_socket = -1;
    for (int i = 0; i < nof_clients; i++)
    {
        if (strcmp(clinets[i].name, whisper_reciver) == 0)
        {
            reciver_socket = clinets[i].socket;
            break;
        }
    }
    if(reciver_socket ) {
        char format_msg[BUFFER_SIZE];
        snprintf(format_msg, BUFFER_SIZE, "%s: %s", client->name, whisper_message);
        send(reciver_socket, format_msg, strlen(format_msg), 0);
    }  
   }
    else {
         pthread_mutex_lock(&clinets_mutex);
         char format_msg[BUFFER_SIZE];
         snprintf(format_msg, BUFFER_SIZE, "%s: %s", client->name, whisper_message);
    for (int i = 0; i < nof_clients; i++) {
        if (clinets[i].socket != client->socket) {
            send(clinets[i].socket, message, strlen(message), 0);
        }
    }
    pthread_mutex_unlock(&clinets_mutex);
    } 
};


void *client_func(void* arg) {
    client_t *client = (client_t *)arg;
    char buffer[BUFFER_SIZE];
    char name[NAME_SIZE]; 
    //get client name
    if (recv(client->socket, name, NAME_SIZE, 0) < 0) {
        perror("recv client name");
        close(client->socket);
        return NULL;
    }
    //Copy the name to the client struct
    strncpy(client->name, name, NAME_SIZE);
    printf("New client %s joined from %s\n", client->name, client->ip);
    
    //Get the message from the client
    while (1) {
        int len = recv(client->socket, buffer, BUFFER_SIZE, 0);
        handle_clinet_message(client, buffer, len);
    }
}

main(int argc, char *argv[]) {
    //Open Socket
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("socket");
        exit(1);
    }
    //Set up the server address struct
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    //Convert port number to network byte order
    server_addr.sin_port = htons(argv[1]);
    //Option to bind to any address
    server_addr.sin_addr.s_addr = INADDR_ANY;
    
    //Bind the socket to the address
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind");
        exit(1);
    }   
    //Listen for incoming connections
    if (listen(server_socket, MAX_CLIENTS) == -1) {
        perror("Listen failed");
        return 1;
    }
    printf("Server listening on port %s\n", argv[1]);

    while (1)
    {
        //Construct new client address struct
        struct sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(client_addr);
        //Accept incoming connections
        int client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_addr_len);
        if (client_socket < 0) {
            perror("accept");
            exit(1);
        }   
        //Check Avalavble Slot
        if (nof_clients >= MAX_CLIENTS) {
            printf("Max clients reached\n");
            close(client_socket);
            continue;
        }
        pthread_mutex_lock(&clinets_mutex);
        //Add client to the list
        client_t *client = &clinets[nof_clients++];
        client->socket = client_socket;
        //Get client IP address
        inet_ntop(AF_INET, &client_addr.sin_addr, client->ip, INET_ADDR_STRLEN);
        pthread_mutex_unlock(&clinets_mutex);

        //Create a new thread to handle the client  
        pthread_t client_thread;
        pthread_create(&client_thread, NULL, client_func, client);

        

    }
    

    return 0;
}