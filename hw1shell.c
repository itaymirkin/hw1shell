#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <string.h>

#define MAX_THREADS 4096
#define MAX_COUNTERS 100
#define MAX_LINE_LENGTH 1024

pthread_t* worker_trds;
pthread_mutex_t work_queue_lock;
pthread_cond_t work_available;

//* Globals *//

int num_threads;          
int num_counters;
int log_enabled;

void *trd_func(void *arg) {
    // Placeholder worker thread logic
    printf("Worker thread running\n");
    return NULL;
}
void proccess_cmd(char *cmd) {
    printf("Worker thread running\n");
    

}



//tokenize cmd line by semi columns 
void dispatcher_command(char *line) {
    char *token;
    token = strtok(line, ";");
    while (token != NULL) {
        while (*token == ' ') token++; //rmv spaces 
        char *end = token + strlen(token) - 1;
        while (end > token && (*end == ' ' || *end == '\n')) {
            *end = '\0';
            end--;
        }

        //proccess cmd line
        proccess_cmd(token);

        // Get the next token
        token = strtok(NULL, ";"); //continue tokenize
    }
}



int main(int argc, char *argv[]) {
    //Validate num of args
    if (argc != 5) {
        fprintf(stderr, "not enough args\n");
        return 1;
    }
    //init main args
    num_threads = atoi(argv[1]);
    num_counters = atoi(argv[2]);
    log_enabled = atoi(argv[3]);
     
    //Create countes files and init them 
    char counter_filename[50];
    for (int i = 0; i < num_counters; i++)
    {
        sprintf(counter_filename, "count%02d.txt", i);
        FILE *f = fopen(counter_filename, "W");
        if (f == NULL)
        {
            printf("FILE CREATION ERROR");
            return 1;
        }
        fprintf(f, "0\n");
        fclose(f);
    }
    



        worker_trds = calloc(num_threads, sizeof(pthread_t));


        for (int i = 0; i < num_threads; i++)
        {
            pthread_create(&worker_trds[i], NULL, trd_func, NULL); // add Pointer with trd DATA struct to last null
        }

        /* Dispatcher Work*/

        //Parse cmd line 
        FILE *cmdfile = fopen(argv[1], "r");
        if (cmdfile == NULL) {
            printf("CANT OPEN CMDFILE");
            return 1;
        }

        char line[MAX_LINE_LENGTH];
        while (fgets(line, MAX_LINE_LENGTH, cmdfile)) {
            //loaded a line
            line[strcspn(line, "\n")] = '\0';
            //tokenize line and excute
            dispatcher_command(line);
    }
    return 0;
}


