#ifndef NETWORK_CHAT_APP_H
#define NETWORK_CHAT_APP_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#define BUFFER_SIZE 1024
#define MAX_CLIENTS 16
#define NAME_SIZE 256
#define INET_ADDR_STRLEN 16

typedef struct {
    int socket;
    char name[NAME_SIZE];
    char ip[INET_ADDR_STRLEN];
    char local_ip[INET_ADDR_STRLEN];
} client_t;

extern client_t clients[MAX_CLIENTS];
extern int nof_clients;
extern pthread_mutex_t clients_mutex;

void *recv_func(void *arg);
void remove_client(client_t *client);
void handle_clinet_message(client_t *client, char *message, int len);
void *client_func(void *arg);

#endif // NETWORK_CHAT_APP_H
