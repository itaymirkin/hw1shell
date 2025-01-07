#include "hw3.h"

client_t clients [MAX_CLIENTS];
int nof_clients = 0;
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;
//Function to remove client
void remove_client(client_t *client) {
    pthread_mutex_lock(&clients_mutex);
    char clinet_name[NAME_SIZE];
    strncpy(clinet_name, client->name, NAME_SIZE);
    //Print the client name disconnected
    printf("client %s disconnected\n", client->name);
    for (int i = 0; i < nof_clients; i++) {
        if (clients[i].socket == client->socket) {
            for (int j = i; j < nof_clients - 1; j++) {
                clients[j] = clients[j + 1];
            }
            nof_clients--;
            break;
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}
//Function to handle client messages
void handle_clinet_message(client_t *client, char *message, int len) {
   //printf("message : %s\n", message); - debug
   if(strcmp(message, "!exit") == 0) {
       remove_client(client);
       close(client->socket);
   }
   if (message[0] == '@')
   {
        char *name, *msg_out;
        // Get the first token after '@'
        name = strtok(message, " ");  // Split at the first space
        if (name != NULL && name[0] == '@') {
            name++;  // Skip the '@' character
        }

        // Get the second part (msg_out)
        msg_out = strtok(NULL, "");  // The rest of the string
        //printf("msg_out : %s\n", msg_out); - debug
        //find socket of the reciver
        int reciver_socket = -1;
        for (int i = 0; i < nof_clients; i++)
        {
            if (strcmp(clients[i].name, name) == 0)
            {
                reciver_socket = clients[i].socket;
                break;
            }
        }
        char format_msg[BUFFER_SIZE];
        memset(format_msg, 0, BUFFER_SIZE);
        
        if(reciver_socket != -1) {
            
            snprintf(format_msg, 2 * BUFFER_SIZE, "%s: %s", client->name, msg_out);
            //printf("format_msg from @ case : %s\n", format_msg);
            send(reciver_socket, format_msg, strlen(format_msg), 0);
        }
        
   }
    else {
        pthread_mutex_lock(&clients_mutex);
        char format_msg[BUFFER_SIZE];
        memset(format_msg, 0, BUFFER_SIZE);
        snprintf(format_msg, 2 * BUFFER_SIZE, "%s: %s", client->name, message);
        //printf("format_msg everybody case : %s\n", format_msg);
        for (int i = 0; i < nof_clients; i++) {
            if (clients[i].socket != client->socket) {
                send(clients[i].socket, format_msg, strlen(format_msg), 0);
            }
        }
        pthread_mutex_unlock(&clients_mutex);
    } 
};



void *client_func(void* arg) {
    client_t *client = (client_t *)arg;
    char buffer[BUFFER_SIZE];
    char name[NAME_SIZE]; 
    //get client name
    memset(buffer, 0, BUFFER_SIZE);
    if (recv(client->socket, name, NAME_SIZE, 0) < 0) {
        perror("recv client name");
        close(client->socket);
        return NULL;
    }
    //Copy the name to the client struct
    strncpy(client->name, name, NAME_SIZE);
    printf("client %s connected from %s\n", client->name, client->local_ip);
    fflush(stdout);
    //Get messages from the client
    while (1) {
        int len = recv(client->socket, buffer, BUFFER_SIZE, 0);
        if (len <= 0) {
            handle_clinet_message(client, "!exit", 5);
            return NULL;
        }
        handle_clinet_message(client, buffer, len);
        memset(buffer, 0, BUFFER_SIZE);
    }
}

int main(int argc, char *argv[]) {
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
    //printf("argv[1] - %s\n", argv[1]);
    server_addr.sin_port = htons(atoi(argv[1]));
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
    // Construct new client address struct
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    // Accept incoming connections
    int client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_addr_len);
    if (client_socket < 0) {
        perror("accept");
        exit(1);
    }   
    // Check Available Slot
    if (nof_clients >= MAX_CLIENTS) {
        printf("Max clients reached\n");
        close(client_socket);
        continue;
    }
    pthread_mutex_lock(&clients_mutex);
    // Add client to the list
    client_t *client = &clients[nof_clients++];
    client->socket = client_socket;
    // Get client IP address
    inet_ntop(AF_INET, &client_addr.sin_addr, client->ip, INET_ADDR_STRLEN);
    //printf("Accepted connection from %s\n", client->ip); // Add this line for debugging
    
    // Get the local socket address (to retrieve the exact IP the client used)
    struct sockaddr_in local_addr;
    socklen_t local_addr_len = sizeof(local_addr);
    if (getsockname(client_socket, (struct sockaddr *)&local_addr, &local_addr_len) == 0) {
        inet_ntop(AF_INET, &local_addr.sin_addr, client->local_ip, INET_ADDR_STRLEN);
    } else {
        perror("getsockname");
    }
    
    pthread_mutex_unlock(&clients_mutex);
    
    // Create a new thread to handle the client  
    pthread_t client_thread;
    if (pthread_create(&client_thread, NULL, client_func, client) != 0) {
        perror("pthread_create");
        close(client_socket);
        continue;
    }
    pthread_detach(client_thread);
}

return 0;
}
