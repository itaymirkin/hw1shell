#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <string.h>
#include "hw2threads_src.h"

// pthread_t* worker_trds;
pthread_mutex_t work_queue_lock;
pthread_cond_t work_available;


//* Globals *//

int num_threads;          
int num_counters;
int log_enabled;

int main(int argc, char *argv[]) {
    //Validate num of args
    if (argc != 5) {
        fprintf(stderr, "not enough args\n");
        return 1;
    }
    //init main args
    int num_threads = atoi(argv[2]);
    int num_counters = atoi(argv[3]);
    int log_enabled = atoi(argv[4]);
    printf("num of counters - %d \nnum of threads - %d\nlog enabled - %d\n", num_counters, num_threads, log_enabled); 

    //Create countes files and init them 
    char counter_filename[50];
    for (int i = 0; i < num_counters; i++)
    {
        sprintf(counter_filename, "count%02d.txt", i);
        printf("file name - %s\n", counter_filename);
        FILE *f = fopen(counter_filename, "w");
        if (f == NULL)
        {
            printf("FILE CREATION ERROR HEREs");
            return 1;
        }
        fprintf(f, "0\n");
        fclose(f);
    }
    


        // Why not just define
        pthread_t worker_trds[num_threads]; 

        // worker_trds = calloc(num_threads, sizeof(pthread_t));


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
            printf("LINE as line : %s\n", line);
            parse_line(line);
    }
    return 0;
}


